// Pyrticle - Particle in Cell in Python
// Reconstruction based on shape functions
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





#ifndef _BADFJAH_PYRTICLE_REC_SHAPE_HPP_INCLUDED
#define _BADFJAH_PYRTICLE_REC_SHAPE_HPP_INCLUDED




#include <boost/foreach.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/typeof/std/utility.hpp>
#include "tools.hpp"
#include "bases.hpp"
#include "meshdata.hpp"




namespace pyrticle 
{
  class shape_function
  {
    public:
      shape_function(double radius, unsigned dimensions, double alpha=2);

      const double operator()(const hedge::vector &r) const
      {
        double r_squared = inner_prod(r, r);
        if (r_squared > m_l_squared)
          return 0;
        else
          return m_normalizer * pow(m_l-r_squared/m_l, m_alpha);
      }

      const double radius() const
      { return m_l; }

    private:
      double m_normalizer;
      double m_alpha;
      double m_l, m_l_squared;
  };




  /** The ReconstructionTarget protocol:
   *
   * template <class Scaler>
   * class reconstruction_target
   * {
   *   void begin_particle(particle_number pn);
   *   void add_shape_at_point(unsigned i, double shape_factor)
   *   void end_particle(particle_number pn)
   * };
   *
   * Note: this is a stateful protocol.
   */

  class rho_reconstruction_target
  {
    private:
      hedge::vector &m_target_vector;
      const hedge::vector &m_charges;
      double m_scale_factor;

    public:
      rho_reconstruction_target(
          hedge::vector &target_vector, const hedge::vector &charges)
        : m_target_vector(target_vector), m_charges(charges)
      { 
        m_target_vector.clear();
      }

      void begin_particle(particle_number pn)
      {
        m_scale_factor = m_charges[pn];
      }

      void add_shape_at_point(unsigned i, double shape_factor)
      {
        m_target_vector[i] += shape_factor * m_scale_factor;
      }

      void end_particle(particle_number pn)
      { }

      const hedge::vector &result() const
      {
        return m_target_vector;
      }
  };




  /** Reconstruction Target for the current density.
   */
  template<unsigned DimensionsVelocity>
  class j_reconstruction_target
  {
    private:
      hedge::vector &m_target_vector;
      const hedge::vector &m_charges;
      const hedge::vector &m_velocities;
      double m_scale_factors[DimensionsVelocity];

    public:
      j_reconstruction_target(
          hedge::vector &target_vector, 
          const hedge::vector &charges,
          const hedge::vector &velocities)
        : m_target_vector(target_vector), 
        m_charges(charges), m_velocities(velocities)
      { 
        m_target_vector.clear();
      }

      void begin_particle(particle_number pn)
      {
        const double charge = m_charges[pn];
        for (unsigned axis = 0; axis < DimensionsVelocity; axis++)
          m_scale_factors[axis] = charge * m_velocities[pn*DimensionsVelocity+axis];
      }

      void add_shape_at_point(unsigned i, double shape_factor)
      {
        const unsigned base = i*DimensionsVelocity;
        for (unsigned axis = 0; axis < DimensionsVelocity; axis++)
          m_target_vector[base+axis] += shape_factor * m_scale_factors[axis];
      }

      void end_particle(particle_number pn)
      { }

      const hedge::vector &result() const
      {
        return m_target_vector;
      }
  };




  template <class T1, class T2>
  class chained_reconstruction_target
  {
    private:
      T1 m_target1;
      T2 m_target2;

    public:
      chained_reconstruction_target(T1 &target1, T2 &target2)
        : m_target1(target1), m_target2(target2)
      { }

      void begin_particle(particle_number pn)
      {
        m_target1.begin_particle(pn);
        m_target2.begin_particle(pn);
      }

      void add_shape_at_point(unsigned i, double shape_factor)
      {
        m_target1.add_shape_at_point(i, shape_factor);
        m_target2.add_shape_at_point(i, shape_factor);
      }

      void end_particle(particle_number pn)
      {
        m_target1.end_particle(pn);
        m_target2.end_particle(pn);
      }

      void end_particle(particle_number pn)
  };



  template <class T1, class T2>
  inline
  chained_reconstruction_target<T1, T2> 
  make_chained_reconstruction_target(T1 &target1, T2 &target2)
  {
    return chained_reconstruction_target<T1, T2>(target1, target2);
  }



  inline
  bool is_not_near_vertex(hedge::vector const &unit_pt)
  {
    bool not_near_vertex = true;

    double uc_sum = 0;
    BOOST_FOREACH(double uc, unit_pt)
    {
      if (uc > 0)
      {
        not_near_vertex = false;
        break;
      }
      uc_sum += uc;
    }

    return not_near_vertex && (1-0.5*(uc_sum+unit_pt.size()) < 0.5);
  }




  template <class PICAlgorithm>
  class shape_element_finder
  {
    public:
      template <class Target>
      void add_shape_by_neighbors(
          Target &target,
          const hedge::vector &pos,
          const mesh_data::element_info &einfo,
          double radius)
      {
        PIC_THIS->add_shape_on_element(target, pos, einfo.m_id);

        BOOST_FOREACH(mesh_data::element_number en, einfo.m_neighbors)
          if (en != mesh_data::INVALID_ELEMENT)
            PIC_THIS->add_shape_on_element(target, pos, en);

        // now check if we need to redo this for periodic copies
        BOOST_FOREACH(const mesh_data::periodicity_axis &pa,
            CONST_PIC_THIS->m_mesh_data.m_periodicities)
        {
          if (pos[pa.m_axis] - radius < pa.m_min)
          {
            hedge::vector pos2(pos);
            pos2[pa.m_axis] += (pa.m_max-pa.m_min);

            BOOST_FOREACH(mesh_data::element_number en, einfo.m_neighbors)
              if (en != mesh_data::INVALID_ELEMENT)
                PIC_THIS->add_shape_on_element(target, pos2, en);
          }
          if (pos[pa.m_axis] + radius > pa.m_max)
          {
            hedge::vector pos2(pos);
            pos2[pa.m_axis] -= (pa.m_max-pa.m_min);

            BOOST_FOREACH(mesh_data::element_number en, einfo.m_neighbors)
              if (en != mesh_data::INVALID_ELEMENT)
                PIC_THIS->add_shape_on_element(target, pos2, en);
          }
        }
      }




      template <class Target>
      void add_shape_by_vertex(
          Target &target,
          const hedge::vector &pos,
          const mesh_data::element_info &einfo,
          double radius)
      {
        // find closest vertex
        mesh_data::vertex_number closest_vertex = 
          mesh_data::INVALID_VERTEX;
        double min_dist = std::numeric_limits<double>::infinity();

        BOOST_FOREACH(mesh_data::vertex_number vi, einfo.m_vertices)
        {
          double dist = norm_2(CONST_PIC_THIS->m_mesh_data.m_vertices[vi] - pos);
          if (dist < min_dist)
          {
            closest_vertex = vi;
            min_dist = dist;
          }
        }

        const mesh_data &md = CONST_PIC_THIS->m_mesh_data;

        // go through vertex-adjacent elements
        BOOST_AUTO(vertex_el_range, 
            std::make_pair(
              md.m_vertex_adj_elements.begin()
              + md.m_vertex_adj_element_starts[closest_vertex],
              md.m_vertex_adj_elements.begin()
              + md.m_vertex_adj_element_starts[closest_vertex+1]
              )
            );
        
        BOOST_FOREACH(mesh_data::element_number en, vertex_el_range)
          PIC_THIS->add_shape_on_element(target, pos, en);

        // now check if we need to redo this for periodic copies
        BOOST_FOREACH(const mesh_data::periodicity_axis &pa,
            md.m_periodicities)
        {
          if (pos[pa.m_axis] - radius < pa.m_min)
          {
            hedge::vector pos2(pos);
            pos2[pa.m_axis] += (pa.m_max - pa.m_min);

            BOOST_FOREACH(mesh_data::element_number en, vertex_el_range)
              PIC_THIS->add_shape_on_element(target, pos2, en);
          }
          if (pos[pa.m_axis] + radius > pa.m_max)
          {
            hedge::vector pos2(pos);
            pos2[pa.m_axis] -= (pa.m_max - pa.m_min);

            BOOST_FOREACH(mesh_data::element_number en, vertex_el_range)
              PIC_THIS->add_shape_on_element(target, pos2, en);
          }
        }
      }




      template <class Target>
      void add_shape(Target &target, particle_number pn, double radius)
      {
        const unsigned dim = CONST_PIC_THIS->get_dimensions_pos();
        const hedge::vector pos = subrange(
            CONST_PIC_THIS->m_positions, pn*dim, (pn+1)*dim);
        const mesh_data::element_number containing_el = 
          CONST_PIC_THIS->m_containing_elements[pn];
        const mesh_data::element_info &einfo(
            CONST_PIC_THIS->m_mesh_data.m_element_info[containing_el]);

        // We're deciding between RULE A and RULE B below.
        // The decision is made by looking at the barycentric coordinates of
        // the center of the shape function. If that center has all barycentric
        // coordinates 1/2 or smaller, then use the faster RULE A.
        //
        // For this purpose, observe that the unit coordinates are the first
        // barycentric coordinates, and the remaining one is found by calculating
        // 1-sum(lambda_i)
        //
        // Note also that this is not purely a speed tradeoff. RULE B works fairly
        // well alone, but falls flat on its face for near the center of the hypotenuse
        // of a right triangle.

        // FIXME this assumes dimension_pos == dimension_mesh

        if (is_not_near_vertex(einfo.m_inverse_map(pos)))
        {
          // RULE A: we're far enough away from vertices, 
          //m_neighbor_shape_adds.tick();
          add_shape_by_neighbors(target, pos, einfo, radius);
        }
        else
        {
          // RULE B: we're close to a vertex, weight onto all elements
          // adjoining that vertex
          //m_vertex_shape_adds.tick();
          add_shape_by_vertex(target, pos, einfo, radius);
        }
      }
  };





  struct shape_function_reconstructor
  {
    template <class PICAlgorithm>
    class type : 
      public shape_element_finder<PICAlgorithm>, 
      public reconstructor_base
    {
      public:
        // member data --------------------------------------------------------
        std::auto_ptr<shape_function>   m_shape_function;

        // public interface ---------------------------------------------------
        static const char *get_name()
        { return "Shape"; }

        void set_radius(double radius)
        {
          m_shape_function = std::auto_ptr<shape_function>(
              new shape_function(radius, CONST_PIC_THIS->m_mesh_data.m_dimensions));
        }




        void reconstruct_densities(
            hedge::vector &rho, 
            hedge::vector &j,
            const hedge::vector &velocities)
        {
          if (rho.size() != CONST_PIC_THIS->m_mesh_data.m_nodes.size())
            throw std::runtime_error("rho field does not have the correct size");
          if (j.size() != CONST_PIC_THIS->m_mesh_data.m_nodes.size() *
              CONST_PIC_THIS->get_dimensions_velocity())
            throw std::runtime_error("j field does not have the correct size");

          rho_reconstruction_target rho_tgt(rho, CONST_PIC_THIS->m_charges);
          j_reconstruction_target<PICAlgorithm::dimensions_velocity> j_tgt(
              j, CONST_PIC_THIS->m_charges, velocities);

          chained_reconstruction_target
            <rho_reconstruction_target, 
            j_reconstruction_target<PICAlgorithm::dimensions_velocity> >
            tgt(rho_tgt, j_tgt);
          reconstruct_densities_on_target(tgt);

          rho = rho_tgt.result();
        }




        void reconstruct_j(hedge::vector &j, const hedge::vector &velocities)
        {
          if (j.size() != CONST_PIC_THIS->m_mesh_data.m_nodes.size() *
              CONST_PIC_THIS->get_dimensions_velocity())
            throw std::runtime_error("j field does not have the correct size");

          j_reconstruction_target<PICAlgorithm::dimensions_velocity> j_tgt(
              j, CONST_PIC_THIS->m_charges, velocities);

          reconstruct_densities_on_target(j_tgt);
        }




        void reconstruct_rho(hedge::vector &rho)
        {
          if (rho.size() != CONST_PIC_THIS->m_mesh_data.m_nodes.size())
            throw std::runtime_error("rho field does not have the correct size");
          rho_reconstruction_target rho_tgt(rho, CONST_PIC_THIS->m_charges);

          reconstruct_densities_on_target(rho_tgt);
        }




        void perform_reconstructor_upkeep()
        { }




        // inner workings -----------------------------------------------------
        template <class Target>
        void add_shape_on_element(
            Target &tgt, 
            const hedge::vector &center,
            const mesh_data::element_number en
            ) const
        {
          const mesh_data::element_info &einfo(
              CONST_PIC_THIS->m_mesh_data.m_element_info[en]);
          for (unsigned i = einfo.m_start; i < einfo.m_end; i++)
            tgt.add_shape_at_point(i, 
                (*m_shape_function)(CONST_PIC_THIS->m_mesh_data.m_nodes[i]-center));
        }




        template<class Target>
        void reconstruct_densities_on_target(Target &tgt)
        {
          for (particle_number pn = 0; pn < CONST_PIC_THIS->m_particle_count; ++pn)
          {
            tgt.begin_particle(pn);
            add_shape(tgt, pn, m_shape_function->radius());
            tgt.end_particle(pn);
          }
        }
    };

  };

}




#endif
