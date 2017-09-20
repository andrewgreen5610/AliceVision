// This file is part of the AliceVision project and is made available under
// the terms of the MPL2 license (see the COPYING.md file).

#pragma once

#include <stdint.h>
//#include <iostream>
#include <Eigen/Core>

namespace aliceVision {
namespace voctree {


/**
 * \brief Default implementation of L2 distance metric.
 *
 * Works with std::vector, boost::array, or more generally any container that has
 * a \c value_type typedef, \c size() and array-indexed element access.
 */
template<class DescriptorA, class DescriptorB=DescriptorA>
struct L2
{
  typedef typename DescriptorA::value_type value_type;
  typedef double result_type;

  result_type operator()(const DescriptorA& a, const DescriptorB& b) const
  {
    result_type result = result_type(0);
    for(std::size_t i = 0; i < a.size(); ++i)
    {
      result_type diff = (result_type)a[i] - (result_type)b[i];
      result += diff*diff;
    }
    return result;
  }
};


/// @todo Version for raw data pointers that knows the size of the feature
/// @todo Specialization for cv::Vec. Doesn't have size() so default won't work.

/// Specialization for Eigen::Matrix types.

template<typename Scalar, int Rows, int Cols, int Options, int MaxRows, int MaxCols>
struct L2< Eigen::Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols>, Eigen::Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols> >
{
  typedef Eigen::Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols> feature_type;
  typedef Scalar value_type;
  typedef double result_type;

  result_type operator()(const feature_type& a, const feature_type& b) const
  {
    return (a - b).squaredNorm();
  }
};

}
}
