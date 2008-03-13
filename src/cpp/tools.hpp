// Pyrticle - Particle in Cell in Python
// Little bits of helpfulness
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





#ifndef _BADFJAH_PYRTICLE_TOOLS_HPP_INCLUDED
#define _BADFJAH_PYRTICLE_TOOLS_HPP_INCLUDED




#include <functional>
#include <hedge/base.hpp>
#include <boost/foreach.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>




namespace pyrticle 
{
  // utilities ----------------------------------------------------------------
  template <class Map>
  typename Map::mapped_type const &map_get(
      Map const &map, 
      typename Map::key_type const &key)
  {
    typename Map::const_iterator it = map.find(key);
    if (it == map.end())
      throw std::logic_error("item not found in map");
    else
      return it->second;
  }




  // common types -------------------------------------------------------------
  typedef unsigned particle_number;
  static const particle_number INVALID_PARTICLE = UINT_MAX;




  // common ublas types -------------------------------------------------------
  namespace ublas = boost::numeric::ublas;

  typedef ublas::compressed_matrix<
    double, ublas::column_major, 0, 
    ublas::unbounded_array<int> >
      csr_matrix;
  typedef ublas::zero_vector<
    hedge::vector::value_type>
    zero_vector;




  class event_counter
  {
    private:
      unsigned          m_count;

    public:
      event_counter()
        : m_count(0)
        { }

      unsigned get()
      { return m_count; }

      unsigned pop()
      { 
        unsigned result = m_count; 
        m_count = 0;
        return result;
      }

      void tick()
      { ++m_count; }
  };




  template <class T>
  inline const T square(T x)
  {
    return x*x;
  }





  template <class VecType>
  inline hedge::vector::value_type entry_or_zero(const VecType &v, int i)
  {
    if (i >= v.size())
      return 0;
    else
      return v[i];
  }




  inline hedge::vector::value_type entry_or_zero(const hedge::vector::value_type *v, int i)
  {
    return v[i];
  }




  template <class VecType1, class VecType2>
  inline
  const hedge::vector cross(
      const VecType1 &a, 
      const VecType2 &b)
  {
    hedge::vector result(3);
    result[0] = entry_or_zero(a,1)*entry_or_zero(b,2) - entry_or_zero(a,2)*entry_or_zero(b,1);
    result[1] = entry_or_zero(a,2)*entry_or_zero(b,0) - entry_or_zero(a,0)*entry_or_zero(b,2);
    result[2] = entry_or_zero(a,0)*entry_or_zero(b,1) - entry_or_zero(a,1)*entry_or_zero(b,0);
    return result;
  }




  template <class T> 
  struct identity : std::unary_function<T,T> {
    const T operator()(const T& x) const
    {
      return x;
    }
  };




  template <class InputIterator, class Function>
  inline double average(InputIterator first, InputIterator last, 
      Function fn = identity<double>())
  {
    double result = 0;
    unsigned count = 0;

    BOOST_FOREACH(double value, std::make_pair(first,last))
    {
      result += fn(value);
      ++count;
    }

    if (count == 0)
      throw std::runtime_error("attempted to take empty average");

    return result/count;
  }





  template <class InputIterator>
  inline double std_dev(InputIterator first, InputIterator last)
  {
    double square_sum = 0;
    double sum = 0;
    unsigned count = 0;

    BOOST_FOREACH(double value, std::make_pair(first,last))
    {
      sum += value;
      square_sum += square(value);
      ++count;
    }

    if (count == 0)
      throw std::runtime_error("attempted to take empty average");

    double mean = sum/count;
    return sqrt(square_sum/count - square(mean));
  }




  template <class T>
  class stats_gatherer
  {
    private:
      unsigned m_count;
      T m_sum, m_square_sum;
      T m_min, m_max;

    public:
      stats_gatherer()
        : m_count(0), m_sum(0), m_square_sum(0)
      { }

      void add(T x)
      {
        m_sum += x;
        m_square_sum += square(x);

        if (m_count == 0 || x < m_min)
          m_min = x;

        if (m_count == 0 || x > m_max)
          m_max = x;

        ++m_count;
      }

      void reset()
      {
        m_count = 0;
        m_sum = 0;
        m_square_sum = 0;
      }

      unsigned count() const
      { return m_count; }

      T minimum() const
      { return m_min; }

      T maximum() const
      { return m_max; }

      T mean() const
      {
        if (m_count == 0)
          throw std::runtime_error("attempted to take empty mean");

        return m_sum/m_count;
      }

      T variance() const
      {
        if (m_count == 0)
          throw std::runtime_error("attempted to take empty mean");

        return m_square_sum/m_count - square(mean());
      }

      T standard_deviation() const
      {
        return sqrt(variance());
      }

  };




  class visualization_listener
  {
    public:
      virtual ~visualization_listener()
      { }

      virtual void store_particle_vis_vector(
          const char *name,
          const hedge::vector &vec,
          unsigned entries_per_particle
          ) const = 0;
  };




  class number_shift_listener
  {
    public:
      virtual ~number_shift_listener()
      { }

      virtual void note_change_size(unsigned new_size) const 
      { }
      virtual void note_move(unsigned orig, unsigned dest, unsigned size) const 
      { }
      virtual void note_reset(unsigned start, unsigned size) const 
      { }
  };



  class warning_listener
  {
    private:
      static warning_listener   *m_singleton;

    public:
      warning_listener()
      {
        if (m_singleton)
          throw std::runtime_error("warning listener singleton already exists");
        m_singleton = this;
      }

      virtual ~warning_listener()
      { 
        m_singleton = 0;
      }

      static void warn(
          std::string const &message,
          std::string const &filename,
          unsigned lineno
          ) 
      {
        if (m_singleton)
          m_singleton->note_warning(message, filename, lineno);
        else
          throw std::runtime_error("warning raised, but no listener registered");
      }

      virtual void note_warning(
          std::string const &message,
          std::string const &filename,
          unsigned lineno
          ) const = 0;
  };




#define WARN(MESSAGE) \
  warning_listener::warn(MESSAGE, __FILE__, __LINE__);
}




#endif
