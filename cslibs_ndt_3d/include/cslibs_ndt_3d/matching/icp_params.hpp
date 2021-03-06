#ifndef CSLIBS_NDT_PARAMS_HPP
#define CSLIBS_NDT_PARAMS_HPP

#include <cslibs_math_3d/linear/transform.hpp>

#include <cslibs_ndt/matching/parameter.hpp>

namespace cslibs_ndt_3d {
namespace matching {
class EIGEN_ALIGN16 ParametersWithICP : public cslibs_ndt::matching::Parameter
{
public:

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    inline ParametersWithICP(const std::size_t                   max_iterations = 100,
                             const std::size_t                   icp_max_iterations = 50,
                             const double                        icp_min_assigned_points = 0.75,
                             const double                        icp_max_distance = 0.5,
                             const double                        rot_eps = 1e-4,
                             const double                        trans_eps = 1e-4,
                             const std::size_t                   step_adjustment_retries = 5,
                             const double                        alpha = 1.0) :
        cslibs_ndt::matching::Parameter(max_iterations, rot_eps, trans_eps, step_adjustment_retries, alpha),
        icp_max_iterations_(icp_max_iterations),
        icp_min_assigned_points_(icp_min_assigned_points),
        icp_max_distance_(icp_max_distance)
    {
    }

    inline std::size_t maxIterationsICP() const
    {
        return icp_max_iterations_;
    }

    inline std::size_t & maxIterationsICP()
    {
        return icp_max_iterations_;
    }

    inline double minAssignedPoints() const
    {
        return icp_min_assigned_points_;
    }

    inline double & minAssignedPoints()
    {
        return icp_min_assigned_points_;
    }

    inline double maxDistanceICP() const
    {
        return icp_max_distance_;
    }

    inline double & maxDistanceICP()
    {
        return icp_max_distance_;
    }

protected:
    std::size_t icp_max_iterations_;
    double      icp_min_assigned_points_;
    double      icp_max_distance_;
};
}
}

#endif // CSLIBS_NDT_PARAMS_HPP
