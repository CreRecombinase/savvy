
#ifndef LIBSAVVY_EIGEN3_VECTOR_HPP
#define LIBSAVVY_EIGEN3_VECTOR_HPP

#include "allele_vector.hpp"

#include <Eigen/Sparse>
#include <Eigen/Dense>
#include <cstddef>

namespace savvy
{
  namespace eigen3
  {
    template <typename T>
    class sparse_vector : public ::Eigen::SparseVector<T>
    {
    public:
      typedef T value_type;
      using ::Eigen::SparseVector<T>::SparseVector;
      T& operator[](std::size_t idx)
      {
        return ::Eigen::SparseVector<T>::coeffRef(idx);
      }

      T operator[](std::size_t idx) const
      {
        return ::Eigen::SparseVector<T>::coeff(idx);
      }
    };


    template <typename T>
    class dense_vector : public ::Eigen::Matrix<T, 1, ::Eigen::Dynamic>
    {
    public:
      typedef T value_type;
      dense_vector()
      {
      }

      dense_vector(std::size_t size)
      {
        dense_vector::resize(size);
      }

      T& operator[](std::size_t idx)
      {
        return ::Eigen::Matrix<T, 1, ::Eigen::Dynamic>::coeffRef(idx);
      }

      const T& operator[](std::size_t idx) const
      {
        return ::Eigen::Matrix<T, 1, ::Eigen::Dynamic>::coeffRef(idx);
      }

      void resize(std::size_t sz)
      {
        std::size_t before_size = ::Eigen::Matrix<T, 1, ::Eigen::Dynamic>::size();
        ::Eigen::Matrix<T, 1, ::Eigen::Dynamic>::resize(sz);
        if (::Eigen::Matrix<T, 1, ::Eigen::Dynamic>::size() > before_size)
        {
          for (std::size_t i = before_size; i < ::Eigen::Matrix<T, 1, ::Eigen::Dynamic>::size(); ++i)
            (*this)[i] = value_type();
        }
      }
    };

    template <typename T>
    using sparse_allele_vector = allele_vector<sparse_vector<T>>;

    template <typename T>
    using dense_allele_vector = allele_vector<dense_vector<T>>;
  }
}
#endif //LIBSAVVY_EIGEN3_VECTOR_HPP