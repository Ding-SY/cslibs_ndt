#ifndef CSLIBS_NDT_3D_CONVERSION_POINTCLOUD_HPP
#define CSLIBS_NDT_3D_CONVERSION_POINTCLOUD_HPP

#include <cslibs_ndt_3d/dynamic_maps/gridmap.hpp>
#include <cslibs_ndt_3d/dynamic_maps/occupancy_gridmap.hpp>

#include <pcl/point_types.h>
#include <pcl/point_cloud.h>

namespace cslibs_ndt_3d {
namespace conversion {
inline void from(
        const cslibs_ndt_3d::dynamic_maps::Gridmap::Ptr &src,
        pcl::PointCloud<pcl::PointXYZI>::Ptr &dst)
{
    using index_t   = std::array<int, 3>;
    using dst_map_t = pcl::PointCloud<pcl::PointXYZI>;
    dst.reset(new dst_map_t());

    std::vector<index_t> indices;
    src->getBundleIndices(indices);

    for (const index_t &bi : indices) {
        if (const auto &b = src->getDistributionBundle(bi)) {
            cslibs_math::statistics::Distribution<3, 3> d;
            for (std::size_t i = 0 ; i < 8 ; ++i)
                d += b->at(i)->getHandle()->data();
            if (d.getN() == 0)
                continue;

            cslibs_math_3d::Point3d mean(d.getMean());
            pcl::PointXYZI p;
            p.x = static_cast<float>(mean(0));
            p.y = static_cast<float>(mean(1));
            p.z = static_cast<float>(mean(2));
            p.intensity = static_cast<float>(d.sampleMean());

            dst->push_back(p);
        }
    }
}

inline void from(
        const cslibs_ndt_3d::dynamic_maps::OccupancyGridmap::Ptr &src,
        pcl::PointCloud<pcl::PointXYZI>::Ptr &dst,
        const cslibs_gridmaps::utility::InverseModel::Ptr &inverse_model,
        const double &threshold = 0.169)
{
    using index_t   = std::array<int, 3>;
    using dst_map_t = pcl::PointCloud<pcl::PointXYZI>;
    dst.reset(new dst_map_t());

    std::vector<index_t> indices;
    src->getBundleIndices(indices);

    for (const index_t &bi : indices) {
        if (const auto &b = src->getDistributionBundle(bi)) {
            cslibs_math::statistics::Distribution<3, 3> d;
            double occupancy = 0.0;

            for (std::size_t i = 0 ; i < 8 ; ++i) {
                occupancy += 0.125 * b->at(i)->getOccupancy(inverse_model);
                if (b->at(i)->getDistribution())
                    d += *(b->at(i)->getDistribution());
            }
            if (d.getN() == 0 || occupancy < threshold)
                continue;

            cslibs_math_3d::Point3d mean(d.getMean());
            pcl::PointXYZI p;
            p.x = static_cast<float>(mean(0));
            p.y = static_cast<float>(mean(1));
            p.z = static_cast<float>(mean(2));
            p.intensity = static_cast<float>(d.sampleMean() * occupancy);

            dst->push_back(p);
        }
    }
}
}
}

#endif // CSLIBS_NDT_3D_CONVERSION_POINTCLOUD_HPP