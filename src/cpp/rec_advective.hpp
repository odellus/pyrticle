// Pyrticle - Particle in Cell in Python
// Reconstruction based on advected shapes
// Copyright (C) 2007 Andreas Kloeckner
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.





#ifndef _AFAYYTAA_PYRTICLE_REC_ADVECTIVE_HPP_INCLUDED
#define _AFAYYTAA_PYRTICLE_REC_ADVECTIVE_HPP_INCLUDED




#include <vector>
#include <numeric>
#include <boost/array.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/bindings/traits/ublas_matrix.hpp>
#include <boost/numeric/bindings/blas/blas3.hpp>
#include <boost/typeof/std/utility.hpp>
#include <boost/unordered_map.hpp>
#include <pyublas/elementwise_op.hpp>
#include <hedge/face_operators.hpp>
#include "tools.hpp"
#include "meshdata.hpp"
#include "rec_target.hpp"
#include "rec_shape.hpp"
#include "element_finder.hpp"




namespace pyrticle
{
  template <class ParticleState>
  struct advective_reconstructor 
  {
    public:
      static const unsigned max_faces = 4;

      // member types -------------------------------------------------------
      struct active_element
      {
        const mesh_data::element_info *m_element_info;
        boost::array<mesh_data::element_number, 
          advective_reconstructor::max_faces> m_connections;
        unsigned m_start_index;
        unsigned m_min_life;

        active_element()
          : m_element_info(0)
        {
          for (unsigned i = 0; i < advective_reconstructor::max_faces; i++)
            m_connections[i] = mesh_data::INVALID_ELEMENT;
        }
      };

      struct advected_particle
      {
        shape_function                m_shape_function;
        std::vector<active_element>   m_elements;

        active_element *find_element(mesh_data::element_number en)
        {
          if (en == mesh_data::INVALID_ELEMENT)
            return 0;
          BOOST_FOREACH(active_element &el, m_elements)
          {
            if (el.m_element_info->m_id == en)
              return &el;
          }
          return 0;
        }

        const active_element *find_element(mesh_data::element_number en) const
        {
          if (en == mesh_data::INVALID_ELEMENT)
            return 0;
          BOOST_FOREACH(const active_element &el, m_elements)
          {
            if (el.m_element_info->m_id == en)
              return &el;
          }
          return 0;
        }

      };




      // particle state for advective ---------------------------------------
      struct advective_state
      {
        unsigned                        active_elements;
        std::vector<unsigned>           freelist;

        std::vector<advected_particle>  advected_particles;
        dyn_vector                      rho;

        boost::shared_ptr<number_shift_listener> rho_dof_shift_listener;



        advective_state()
          : active_elements(0)
        { }


        void resize(unsigned new_size)
        {
          unsigned old_size = rho.size();
          unsigned copy_size = std::min(new_size, old_size);

          dyn_vector new_rho(new_size);
          subrange(new_rho, 0, copy_size) = subrange(rho, 0, copy_size);
          new_rho.swap(rho);
        }


        void clear()
        {
          advected_particles.clear();
          freelist.clear();
          active_elements = 0;
        }
      };




      // member data --------------------------------------------------------
      const mesh_data                 &m_mesh_data;

      unsigned                        m_faces_per_element;
      unsigned                        m_dofs_per_element;

      py_matrix                       m_mass_matrix;
      dyn_vector                      m_integral_weights;
      py_matrix                       m_inverse_mass_matrix;
      py_matrix                       m_face_mass_matrix;
      dyn_vector                      m_face_integral_weights;
      py_matrix                       m_filter_matrix;

      std::vector<py_matrix>          m_local_diff_matrices;

      boost::shared_ptr<hedge::face_group> m_int_face_group;
      boost::shared_ptr<hedge::face_group> m_bdry_face_group;

      struct face_pair_locator
      {
        hedge::face_group     const *m_face_group;
        hedge::face_pair      const *m_face_pair;

        face_pair_locator()
          : m_face_group(0), m_face_pair(0)
        { }
        face_pair_locator(
            hedge::face_group     const &face_group,
            hedge::face_pair      const &face_pair
            )
          : m_face_group(&face_group), m_face_pair(&face_pair)
        { }

      };

      boost::unordered_map<mesh_data::el_face, face_pair_locator> m_el_face_to_face_pair_locator;

      event_counter m_element_activation_counter, m_element_kill_counter;

      double m_activation_threshold;
      double m_kill_threshold;
      double m_upwind_alpha;




      // initialization -----------------------------------------------------
      advective_reconstructor(
          unsigned faces_per_element, 
          unsigned dofs_per_element,
          const py_matrix &mass_matrix,
          const py_matrix &inverse_mass_matrix,
          const py_matrix &filter_matrix,
          const py_matrix &face_mass_matrix,
          boost::shared_ptr<hedge::face_group> int_face_group,
          boost::shared_ptr<hedge::face_group> bdry_face_group,
          double activation_threshold,
          double kill_threshold,
          double upwind_alpha
          )
        : m_faces_per_element(0), m_dofs_per_element(0), 
        m_activation_threshold(0), m_kill_threshold(0),
        m_upwind_alpha(1)
      {
        m_faces_per_element = faces_per_element;
        m_dofs_per_element = dofs_per_element;

        m_mass_matrix = mass_matrix;
        m_integral_weights = prod(m_mass_matrix, 
            boost::numeric::ublas::scalar_vector<double>
            (m_mass_matrix.size1(), 1));
        m_inverse_mass_matrix = inverse_mass_matrix;

        m_filter_matrix = filter_matrix;

        m_face_mass_matrix = face_mass_matrix;
        m_face_integral_weights = prod(m_face_mass_matrix, 
            boost::numeric::ublas::scalar_vector<double>
            (m_face_mass_matrix.size1(), 1));

        m_int_face_group = int_face_group;
        m_bdry_face_group = bdry_face_group;

        // build m_el_face_to_face_pair_locator
        BOOST_FOREACH(const hedge::face_pair &fp, int_face_group->face_pairs)
        {
          const hedge::fluxes::face &f = fp.loc;
          m_el_face_to_face_pair_locator
            [std::make_pair(f.element_id, f.face_id)] = 
            face_pair_locator(*int_face_group, fp);

          const hedge::fluxes::face &opp_f = fp.opp;
          m_el_face_to_face_pair_locator
            [std::make_pair(opp_f.element_id, opp_f.face_id)] = 
            face_pair_locator(*int_face_group, fp);
        }

        BOOST_FOREACH(const hedge::face_pair &fp, bdry_face_group->face_pairs)
        {
          const hedge::fluxes::face &f = fp.loc;
          m_el_face_to_face_pair_locator
            [std::make_pair(f.element_id, f.face_id)] = 
            face_pair_locator(*bdry_face_group, fp);
        }

        m_activation_threshold = activation_threshold;
        m_kill_threshold = kill_threshold;
        m_upwind_alpha = upwind_alpha;
      }




      void add_local_diff_matrix(unsigned coordinate, const py_matrix &dmat)
      {
        if (coordinate != m_local_diff_matrices.size())
          throw std::runtime_error("local diff matrices added out of order");

        m_local_diff_matrices.push_back(dmat);
      }




      // convenience ----------------------------------------------------------
      unsigned get_dimensions_mesh() const
      { return m_mesh_data.m_dimensions; }




      // main driver ----------------------------------------------------------
      template<class Target>
      void reconstruct_densities_on_target(
          const ParticleState &ps, 
          const advective_state &as, 
          Target &tgt, boost::python::slice const &pslice)
      {
        FOR_ALL_SLICE_INDICES(particle_number, pn, 
            pslice, ps.particle_count)
        {
          tgt.begin_particle(pn);
          BOOST_FOREACH(const active_element &el, 
              as.advected_particles[pn].m_elements)
            tgt.add_shape_on_element(
                el.m_element_info->m_id,
                el.m_element_info->m_start,
                subrange(as.rho, el.m_start_index, el.m_start_index+m_dofs_per_element));
          tgt.end_particle(pn);
        }
      }




      py_vector get_debug_quantity_on_mesh(
          ParticleState &ps,
          const std::string &qty, 
          py_vector const &velocities)
      {
        if (qty == "rhs")
          return map_particle_space_to_mesh_space(
            get_advective_particle_rhs(velocities));
        if (qty == "active_elements")
          return get_active_elements();
        else if (qty == "fluxes")
          return map_particle_space_to_mesh_space(
              calculate_fluxes(velocities));
        else if (qty == "minv_fluxes")
          return map_particle_space_to_mesh_space(
              apply_elementwise_inverse_mass_matrix(
              calculate_fluxes(velocities)));
        else if (qty == "local_div")
          return map_particle_space_to_mesh_space(
              calculate_local_div(velocities));
        else
          throw std::runtime_error("invalid debug quantity");
      }




      void perform_reconstructor_upkeep(
          ParticleState &ps,
          advective_state &as
          )
      {
        // retire empty particle subelements 
        if (m_kill_threshold == 0)
          throw std::runtime_error("zero kill threshold");

        particle_number pn = 0;
        BOOST_FOREACH(advected_particle &p, ps.advected_particles)
        {
          double particle_charge = fabs(ps.charges[pn]);
          for (unsigned i_el = 0; i_el < p.m_elements.size(); ++i_el)
          {
            active_element &el = p.m_elements[i_el];

            if (el.m_min_life)
              --el.m_min_life;

            const double element_charge = element_l1(
                el.m_element_info->m_jacobian,
                subrange(
                  as.rho,
                  el.m_start_index,
                  el.m_start_index+m_dofs_per_element));

            if (el.m_min_life == 0  && element_charge / particle_charge < m_kill_threshold)
            {
              // retire this element
              const hedge::element_number_t en = el.m_element_info->m_id;

              // kill connections
              for (hedge::face_number_t fn = 0; fn < m_faces_per_element; ++fn)
              {
                hedge::element_number_t connected_en = el.m_connections[fn];
                if (connected_en != hedge::INVALID_ELEMENT)
                {
                  active_element &connected_el = *p.find_element(connected_en);

                  for (hedge::face_number_t cfn = 0; cfn < m_faces_per_element; ++cfn)
                  {
                    if (connected_el.m_connections[cfn] == en)
                      connected_el.m_connections[cfn] = hedge::INVALID_ELEMENT;
                  }
                }
              }

              deallocate_element(el.m_start_index);

              // kill the element
              p.m_elements.erase(p.m_elements.begin()+i_el);
            }
            else
              ++i_el;
          }

          ++pn;
        }
      }




      void note_move(ParticleState &ps, advective_state &as,
          particle_number from, particle_number to, unsigned size)
      {
        for (unsigned i = 0; i < size; ++i)
        {
          BOOST_FOREACH(active_element &el, 
              as.advected_particles[to+i].m_elements)
            deallocate_element(el.m_start_index);

          as.advected_particles[to+i] = as.advected_particles[from+i];
        }
      }




      void note_change_size(
          advective_state &as, 
          unsigned particle_count)
      {
        as.advected_particles.resize(particle_count);
      }




      // initialization -----------------------------------------------------
      void dump_particle(advected_particle const &p) const
      {
        std::cout << "particle, radius " << p.m_shape_function.radius() << std::endl;
        unsigned i_el = 0;
        BOOST_FOREACH(const active_element &el, p.m_elements)
        {
          std::cout << "#" << el.m_element_info->m_id << " cnx:(";
          for (unsigned fn = 0; fn < m_faces_per_element; ++fn)
            if (el.m_connections[fn] == hedge::INVALID_ELEMENT)
              std::cout << "X" << ',';
            else
              std::cout << el.m_connections[fn]  << ',';

          std::cout << ")" << std::endl;

          ++i_el;
        }
      }




      // vectors space administration ---------------------------------------
      /* Each element occupies a certain index range in the state
       * vector rho (as well as elsewhere). These functions perform
       * allocation and deallocation of space in these vectors.
       */

      /** Allocate a space for a new element in the state vector, return
       * the start index.
       */
      unsigned allocate_element(advective_state &as)
      {
        if (m_dofs_per_element == 0)
          throw std::runtime_error("tried to allocate element on uninitialized advection reconstructor");

        m_element_activation_counter.tick();

        if (as.freelist.size())
        {
          ++as.active_elements;
          unsigned result = as.freelist.back();
          as.freelist.pop_back();
          return result*m_dofs_per_element;
        }

        // there are no gaps available.
        // return the past-end spot in the array, reallocate if necessary.
        unsigned avl_space = as.rho.size() / m_dofs_per_element;

        if (as.active_elements == avl_space)
        {
          as.resize(2*as.rho.size());
          if (as.rho_dof_shift_listener.get())
            as.rho_dof_shift_listener->note_change_size(as.rho.size());
        }

        return (as.active_elements++)*m_dofs_per_element;
      }


      void deallocate_element(advective_state &as, unsigned start_index)
      {
        if (start_index % m_dofs_per_element != 0)
          throw std::runtime_error("invalid advective element deallocation");

        const unsigned el_index = start_index/m_dofs_per_element;
        --as.active_elements;

        m_element_kill_counter.tick();

        // unless we're deallocating the last element, add it to the freelist.
        if (el_index != as.active_elements+as.freelist.size())
          as.freelist.push_back(el_index);

        if (as.rho_dof_shift_listener.get())
          as.rho_dof_shift_listener->note_reset(start_index, m_dofs_per_element);
      }



      template <class VecType>
      py_vector map_particle_space_to_mesh_space(
          const advective_state &as,
          VecType const &pspace) const
      {
        py_vector result(
            m_dofs_per_element*m_mesh_data.m_element_info.size());
        result.clear();

        BOOST_FOREACH(const advected_particle &p, as.advected_particles)
        {
          BOOST_FOREACH(const active_element &el, p.m_elements)
          {
            const mesh_data::element_info &einfo = *el.m_element_info;
            noalias(subrange(result, einfo.m_start, einfo.m_end)) +=
              subrange(pspace, el.m_start_index, el.m_start_index+m_dofs_per_element);
          }
        }
        
        return result;
      }




      py_vector get_active_elements(
          ParticleState const &ps,
          const advective_state &as
          ) const
      {
        py_vector result(
            m_dofs_per_element*m_mesh_data.m_element_info.size());
        result.clear();

        BOOST_FOREACH(const advected_particle &p, as.advected_particles)
        {
          BOOST_FOREACH(const active_element &el, p.m_elements)
          {
            const mesh_data::element_info &einfo = *el.m_element_info;
            noalias(subrange(result, einfo.m_start, einfo.m_end)) +=
              boost::numeric::ublas::scalar_vector<double>(m_dofs_per_element, 1);
          }
        }
        
        return result;
      }




      // particle construction ----------------------------------------------
      unsigned count_advective_particles(ParticleState const &ps) const
      {
        return ps.advected_particles.size();
      }




    private:
      struct advected_particle_element_target
      {
        private:
          advective_reconstructor       &m_reconstructor;
          advected_particle             &m_particle;

        public:
          advected_particle_element_target(
              advective_reconstructor &rec, advected_particle &p)
            : m_reconstructor(rec), m_particle(p)
          { }

          void add_shape_on_element(
              const bounded_vector &center,
              const mesh_data::element_number en
              )
          {
            const mesh_data::element_info &einfo(
                m_reconstructor.m_mesh_data.m_element_info[en]);

            active_element new_element;
            new_element.m_element_info = &einfo;
            unsigned start = new_element.m_start_index = m_reconstructor.allocate_element();
            new_element.m_min_life = 0;

            for (unsigned i = 0; i < m_reconstructor.m_dofs_per_element; ++i)
              m_reconstructor.m_rho[start+i] =
                m_particle.m_shape_function(
                    m_reconstructor.m_mesh_data.mesh_node(einfo.m_start+i)-center);

            m_particle.m_elements.push_back(new_element);
          }
      };




    public:
      void add_advective_particle(
          const ParticleState &ps,
          advective_state &as,
          shape_function sf, particle_number pn)
      {
        if (pn != as.advected_particles.size())
          throw std::runtime_error("advected particle added out of sequence");

        advected_particle new_particle;
        new_particle.m_shape_function = sf;

        element_finder el_finder(m_mesh_data);

        advected_particle_element_target el_tgt(*this, new_particle);
        el_finder(el_tgt, pn, sf.radius());

        // make connections
        BOOST_FOREACH(active_element &el, new_particle.m_elements)
        {
          const mesh_data::element_info &einfo = *el.m_element_info;

          unsigned fn = 0;
          BOOST_FOREACH(const mesh_data::face_info &f, einfo.m_faces)
          {
            if (new_particle.find_element(f.m_neighbor))
              el.m_connections[fn] = f.m_neighbor;
            ++fn;
          }
        }

        // scale so the amount of charge is correct
        std::vector<double> unscaled_masses;
        BOOST_FOREACH(active_element &el, new_particle.m_elements)
          unscaled_masses.push_back(element_integral(
                el.m_element_info->m_jacobian,
                subrange(as.rho, 
                  el.m_start_index, 
                  el.m_start_index+m_dofs_per_element)));

        const double charge = ps.charges[pn];
        const double total_unscaled_mass = std::accumulate(
            unscaled_masses.begin(), unscaled_masses.end(), double(0));

        double scale;
        if (total_unscaled_mass == 0)
        {
          WARN(str(boost::format("reconstructed initial particle mass is zero"
                  "(particle %d, #elements=%d)") % pn % new_particle.m_elements.size()));
          scale = charge;
        }
        else
          scale = charge / total_unscaled_mass;

        BOOST_FOREACH(active_element &el, new_particle.m_elements)
          subrange(as.rho, 
              el.m_start_index, 
              el.m_start_index+m_dofs_per_element) 
          *= scale;

        as.advected_particles.push_back(new_particle);
      }




      // rhs calculation ----------------------------------------------------
      py_vector calculate_local_div(
          const ParticleState &ps,
          advective_state &as,
          py_vector const &velocities) const
      {
        const unsigned dofs = as.rho.size();
        const unsigned active_contiguous_elements = 
          as.active_elements + as.freelist.size();

        py_vector local_div(dofs);
        local_div.clear();

        // calculate local rst derivatives ----------------------------------
        dyn_vector rst_derivs(get_dimensions_mesh()*dofs);
        rst_derivs.clear();
        using namespace boost::numeric::bindings;
        using blas::detail::gemm;

        for (unsigned loc_axis = 0; loc_axis < get_dimensions_mesh(); ++loc_axis)
        {
          const py_matrix &matrix = m_local_diff_matrices.at(loc_axis);

          gemm(
              'T', // "matrix" is row-major
              'N', // a contiguous array of vectors is column-major
              matrix.size1(),
              active_contiguous_elements,
              matrix.size2(),
              /*alpha*/ 1,
              /*a*/ traits::matrix_storage(matrix.as_ublas()), 
              /*lda*/ matrix.size2(),
              /*b*/ traits::vector_storage(as.rho), 
              /*ldb*/ m_dofs_per_element,
              /*beta*/ 1,
              /*c*/ traits::vector_storage(rst_derivs) + loc_axis*dofs, 
              /*ldc*/ m_dofs_per_element
              );
        }

        // combine them into local part of dot(v, grad rho) -----------------
        {
          particle_number pn = 0;
          BOOST_FOREACH(const advected_particle &p, as.advected_particles)
          {
            bounded_vector v = subrange(velocities, 
                ps.vdim()*pn, ps.vdim()*(pn+1));

            BOOST_FOREACH(const active_element &el, p.m_elements)
            {
              for (unsigned loc_axis = 0; loc_axis < get_dimensions_mesh(); ++loc_axis)
              {
                double coeff = 0;
                for (unsigned glob_axis = 0; glob_axis < get_dimensions_mesh(); ++glob_axis)
                  coeff += -v[glob_axis] *
                    el.m_element_info->m_inverse_map.matrix()(loc_axis, glob_axis);

                subrange(local_div,
                    el.m_start_index,
                    el.m_start_index + m_dofs_per_element) += coeff *
                  subrange(rst_derivs,
                      loc_axis*dofs + el.m_start_index,
                      loc_axis*dofs + el.m_start_index + m_dofs_per_element);
              }
            }
            ++pn;
          }
        }

        return local_div;
      }




      py_vector calculate_fluxes(
          const ParticleState &ps,
          advective_state &as,
          py_vector const &velocities)
      {
        if (m_activation_threshold == 0)
          throw std::runtime_error("zero activation threshold");

        py_vector fluxes(as.rho.size());
        fluxes.clear();

        particle_number pn = 0;
        BOOST_FOREACH(advected_particle &p, as.advected_particles)
        {
          const double shape_peak = 
            p.m_shape_function(
                boost::numeric::ublas::zero_vector<double>(get_dimensions_mesh()))
            * ps.charges[pn];

          const bounded_vector v = subrange(velocities, 
              ps.vdim()*pn, ps.vdim()*(pn+1));

          for (unsigned i_el = 0; i_el < p.m_elements.size(); ++i_el)
          {
            active_element const *el = &p.m_elements[i_el];

            for (hedge::face_number_t fn = 0; fn < m_faces_per_element; ++fn)
            {
              const mesh_data::element_number en = el->m_element_info->m_id;

              /* Find correct fluxes::face instance
               *
               * A face_pair represents both sides of a face. It points
               * to one or two hedge::fluxes::face instances in its face_group that
               * carry information about each side of the face.
               *
               * The "opp" side of the face_pair may be unpopulated because
               * of a boundary.
               *
               * First, we need to identify which side of the face (en,fn)
               * identifies, guarding against an unpopulated "opp" side.
               */
              bool is_boundary;
              const hedge::face_pair::side *flux_face;
              const hedge::face_pair::side *opposite_flux_face;
              hedge::index_lists_t::const_iterator idx_list;
              hedge::index_lists_t::const_iterator opp_idx_list;

              {
                const face_pair_locator &fp_locator = map_get(
                    m_el_face_to_face_pair_locator,
                    std::make_pair(en, fn));

                const hedge::face_group &fg(*fp_locator.m_face_group);
                const hedge::face_pair &fp(*fp_locator.m_face_pair);

                const hedge::face_pair::side &flux_face_a = fp.loc;

                const bool is_face_b = en != flux_face_a.element_id;

                is_boundary = 
                  fp.opp.element_id == hedge::INVALID_ELEMENT;

                const hedge::face_pair::side *flux_face_b = &fp.opp;

                if (is_boundary && is_face_b)
                  throw std::runtime_error("looking for non-existant cross-boundary element");

                if (is_face_b && en != flux_face_b->element_id)
                  throw std::runtime_error("el/face lookup failed");

                flux_face = is_face_b ? flux_face_b : &flux_face_a;
                opposite_flux_face = is_face_b ? &flux_face_a : flux_face_b;

                idx_list = fg.index_list(flux_face->face_index_list_number);
                opp_idx_list = fg.index_list(opposite_flux_face->face_index_list_number);
              }

              // Find information about this face
              const double n_dot_v = inner_prod(v, flux_face->normal);
              const bool inflow = n_dot_v <= 0;
              bool active = el->m_connections[fn] != mesh_data::INVALID_ELEMENT;

              if (is_boundary && active)
                throw std::runtime_error("detected boundary non-connection as active");

              const double int_coeff = 
                flux_face->face_jacobian*(-n_dot_v)*(
                    m_upwind_alpha*(1 - (inflow ? 0 : 1))
                    +
                    (1-m_upwind_alpha)*0.5);
              const double ext_coeff = 
                flux_face->face_jacobian*(-n_dot_v)*(
                    m_upwind_alpha*-(inflow ? 1 : 0)
                    +
                    (1-m_upwind_alpha)*-0.5);

              const mesh_data::node_number this_base_idx = el->m_start_index;

              // activate outflow, if necessary -----------------------------
              if (!is_boundary && !active && !inflow)
              {
                const unsigned face_length = m_face_mass_matrix.size1();

                double max_density = 0;
                for (unsigned i = 0; i < face_length; i++)
                  max_density = std::max(max_density, 
                      fabs(as.rho[this_base_idx+idx_list[i]]));

                // std::cout << max_density << ' ' << shape_peak << std::endl;
                if (max_density > m_activation_threshold*fabs(shape_peak))
                {
                  // yes, activate the opposite element

                  const hedge::element_number_t opp_en = opposite_flux_face->element_id;

                  const mesh_data::element_info &opp_einfo(
                      m_mesh_data.m_element_info[opp_en]);

                  active_element opp_element;
                  opp_element.m_element_info = &opp_einfo;

                  unsigned start = opp_element.m_start_index = allocate_element();
                  subrange(as.rho, start, start+m_dofs_per_element) = 
                    boost::numeric::ublas::zero_vector<double>(m_dofs_per_element);

                  if (as.rho.size() != fluxes.size())
                  {
                    // allocate_element enlarged the size of the state vector
                    // fluxes needs to be changed as well.
                    py_vector new_fluxes(as.rho.size());
                    new_fluxes.clear();
                    noalias(subrange(new_fluxes, 0, fluxes.size())) = fluxes;
                    fluxes.swap(new_fluxes);
                  }

                  opp_element.m_min_life = 10;

                  // update connections
                  hedge::face_number_t opp_fn = 0;
                  BOOST_FOREACH(const mesh_data::face_info &opp_face, opp_einfo.m_faces)
                  {
                    const hedge::element_number_t opp_neigh_en = opp_face.m_neighbor;
                    active_element *opp_neigh_el = p.find_element(opp_neigh_en);
                    if (opp_neigh_el)
                    {
                      /* We found an active neighbor of our "opposite" element.
                       *
                       * Notation:
                       *        *
                       *       / \
                       *      /opp_neigh
                       *     *-----*
                       *    / \opp/
                       *   / el\ /
                       *  *-----*
                       *
                       * el: The element currently under consideration in the 
                       *   "big" loop.
                       * opp: The "opposite" element that we just decided to
                       *   activate.
                       * opp_neigh: Neighbor of opp, also part of this 
                       *   advected_particle
                       */

                       // First, tell opp that opp_neigh exists.
                      opp_element.m_connections[opp_fn] = opp_neigh_en;

                      // Next, tell opp_neigh that opp exists.
                      const mesh_data::element_info &opp_neigh_einfo(
                          m_mesh_data.m_element_info[opp_neigh_en]);

                      mesh_data::face_number opp_index_in_opp_neigh = 0;
                      for (;opp_index_in_opp_neigh < opp_neigh_einfo.m_faces.size()
                          ;++opp_index_in_opp_neigh)
                        if (opp_neigh_einfo.m_faces[opp_index_in_opp_neigh].m_neighbor
                            == opp_en)
                          break;

                      if (opp_index_in_opp_neigh == opp_neigh_einfo.m_faces.size())
                        throw std::runtime_error("opp not found in opp_neigh");

                      opp_neigh_el->m_connections[opp_index_in_opp_neigh] = opp_en;
                    }

                    ++opp_fn;
                  }

                  p.m_elements.push_back(opp_element);

                  // modification of m_elements might have invalidated el,
                  // refresh it

                  el = &p.m_elements[i_el];

                  active = true;
                }
              }
              
              // treat fluxes between active elements -----------------------
              if (active)
              {
                const active_element *opp_el = p.find_element(el->m_connections[fn]);
                const mesh_data::node_number opp_base_idx = opp_el->m_start_index;

                if (opp_el == 0)
                {
                  dump_particle(p);
                  throw std::runtime_error(
                      str(boost::format("opposite element %d of (el:%d,face:%d) for active connection not found")
                      % el->m_connections[fn] % en % fn).c_str());
                }

                const unsigned face_length = m_face_mass_matrix.size1();

                for (unsigned i = 0; i < face_length; i++)
                {
                  const int ili = this_base_idx+idx_list[i];

                  hedge::index_lists_t::const_iterator ilj_iterator = idx_list;
                  hedge::index_lists_t::const_iterator oilj_iterator = opp_idx_list;

                  double res_ili_addition = 0;

                  for (unsigned j = 0; j < face_length; j++)
                  {
                    const double fmm_entry = m_face_mass_matrix(i, j);

                    const int ilj = this_base_idx+*ilj_iterator++;
                    const int oilj = opp_base_idx+*oilj_iterator++;

                    res_ili_addition += 
                      as.rho[ilj]*int_coeff*fmm_entry
                      +as.rho[oilj]*ext_coeff*fmm_entry;
                  }

                  fluxes[ili] += res_ili_addition;
                }
              }

              // handle zero inflow from inactive neighbors -----------------
              else if (inflow)
              {
                const unsigned face_length = m_face_mass_matrix.size1();

                for (unsigned i = 0; i < face_length; i++)
                {
                  const int ili = this_base_idx+idx_list[i];

                  hedge::index_lists_t::const_iterator ilj_iterator = idx_list;

                  double res_ili_addition = 0;

                  for (unsigned j = 0; j < face_length; j++)
                    res_ili_addition += as.rho[this_base_idx+*ilj_iterator++]
                      *int_coeff
                      *m_face_mass_matrix(i, j);

                  fluxes[ili] += res_ili_addition;
                }
              }
            }

          }
          ++pn;
        }

        return fluxes;
      }




      py_vector apply_elementwise_inverse_mass_matrix(
          advective_state &as,
          py_vector const &operand) const
      {
        py_vector result(as.rho.size());
        result.clear();

        const unsigned active_contiguous_elements = 
          as.active_elements + as.freelist.size();

        using namespace boost::numeric::bindings;
        using blas::detail::gemm;

        const py_matrix &matrix = m_inverse_mass_matrix;
        gemm(
            'T', // "matrix" is row-major
            'N', // a contiguous array of vectors is column-major
            matrix.size1(),
            active_contiguous_elements,
            matrix.size2(),
            /*alpha*/ 1,
            /*a*/ traits::matrix_storage(matrix.as_ublas()), 
            /*lda*/ matrix.size2(),
            /*b*/ traits::vector_storage(operand), 
            /*ldb*/ m_dofs_per_element,
            /*beta*/ 1,
            /*c*/ traits::vector_storage(result),
            /*ldc*/ m_dofs_per_element
            );

        // perform jacobian scaling
        BOOST_FOREACH(const advected_particle &p, as.advected_particles)
          BOOST_FOREACH(const active_element &el, p.m_elements)
          {
            subrange(result, 
                el.m_start_index, 
                el.m_start_index+m_dofs_per_element) *= 
            1/el.m_element_info->m_jacobian;
          }

        return result;
      }




      py_vector get_advective_particle_rhs(py_vector const &velocities)
      {
        // calculate_fluxes may resize the state vector--calculate it first,
        // everything else later.
        py_vector fluxes = calculate_fluxes(velocities);

        return calculate_local_div(velocities) 
        - apply_elementwise_inverse_mass_matrix(fluxes);
      }
      



      void apply_advective_particle_rhs(
          const ParticleState &ps,
          advective_state &as,
          py_vector const &rhs)
      {
        if (m_filter_matrix.size1() && m_filter_matrix.size2())
        {
          using namespace boost::numeric::bindings;
          using blas::detail::gemm;

          const unsigned active_contiguous_elements = 
            as.active_elements + as.freelist.size();

          const py_matrix &matrix = m_filter_matrix;
          gemm(
              'T', // "matrix" is row-major
              'N', // a contiguous array of vectors is column-major
              matrix.size1(),
              active_contiguous_elements,
              matrix.size2(),
              /*alpha*/ 1,
              /*a*/ traits::matrix_storage(matrix.as_ublas()), 
              /*lda*/ matrix.size2(),
              /*b*/ traits::vector_storage(rhs), 
              /*ldb*/ m_dofs_per_element,
              /*beta*/ 1,
              /*c*/ traits::vector_storage(as.rho),
              /*ldc*/ m_dofs_per_element
              );
        }
        else
          as.rho += rhs;
      }




      template <class VectorExpression>
      double element_integral(double jacobian, const VectorExpression &ve)
      {
        return jacobian * inner_prod(m_integral_weights, ve);
      }




      template <class VectorExpression>
      double element_l1(double jacobian, const VectorExpression &ve)
      {
        return jacobian * inner_prod(m_integral_weights, 
            pyublas::unary_op<pyublas::unary_ops::fabs>::apply(ve));
      }
  };
}




#endif
