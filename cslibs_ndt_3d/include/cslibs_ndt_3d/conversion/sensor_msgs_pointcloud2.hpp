#ifndef CSLIBS_NDT_3D_CONVERSION_SENSOR_MSGS_POINTCLOUD2_HPP
#define CSLIBS_NDT_3D_CONVERSION_SENSOR_MSGS_POINTCLOUD2_HPP

#include <cslibs_ndt_3d/dynamic_maps/gridmap.hpp>
#include <cslibs_ndt_3d/dynamic_maps/occupancy_gridmap.hpp>
#include <cslibs_ndt_3d/static_maps/gridmap.hpp>
#include <cslibs_ndt_3d/static_maps/occupancy_gridmap.hpp>

#include <sensor_msgs/PointCloud2.h>

namespace cslibs_ndt_3d {
namespace conversion {
inline void from(
        const std::vector<float> &tmp,
        sensor_msgs::PointCloud2 &dst)
{
    // metadata
    dst.width        = tmp.size() / 4;
    dst.height       = 1;
    dst.is_dense     = false;
    dst.is_bigendian = false;
    dst.point_step   = 4 * sizeof(float);
    dst.row_step     = static_cast<uint32_t>(sizeof(float) * tmp.size());

    // fields x y z intensity
    dst.fields.resize(4);
    dst.fields[0].name = "x";
    dst.fields[1].name = "y";
    dst.fields[2].name = "z";
    dst.fields[3].name = "intensity";
    for (int i = 0; i < 4; ++i) {
        dst.fields[i].offset   = i * sizeof(float);
        dst.fields[i].datatype = sensor_msgs::PointField::FLOAT32;
        dst.fields[i].count    = dst.width;
    }

    // data
    std::size_t data_size = sizeof(float) * tmp.size();
    dst.data.resize(data_size);
    memcpy(&dst.data[0], &tmp[0], data_size);
}

template<typename ndt_t,
         typename = typename std::enable_if<std::is_same<ndt_t, cslibs_ndt_3d::dynamic_maps::Gridmap>::value
                                            || std::is_same<ndt_t, cslibs_ndt_3d::static_maps::Gridmap>::value>::type>
inline void from(
        ndt_t &src,
        sensor_msgs::PointCloud2 &dst)
{
    src.allocatePartiallyAllocatedBundles();

    using index_t = std::array<int, 3>;
    using point_t = cslibs_math_3d::Point3d;
    using distribution_t = typename ndt_t::distribution_t;
    using distribution_bundle_t = typename ndt_t::distribution_bundle_t;
    auto sample = [](const distribution_t *d,
                     const point_t &p) -> double {
        return d ? d->data().sampleNonNormalized(p) : 0.0;
    };
    auto sample_bundle = [&sample](const distribution_bundle_t &b,
                                   const point_t &p) -> double {
        return 0.125 * (sample(b.at(0), p) +
                        sample(b.at(1), p) +
                        sample(b.at(2), p) +
                        sample(b.at(3), p) +
                        sample(b.at(4), p) +
                        sample(b.at(5), p) +
                        sample(b.at(6), p) +
                        sample(b.at(7), p));
    };

    std::vector<float> tmp;
    auto process_bundle = [&src, &tmp, &sample_bundle](const index_t &bi, const distribution_bundle_t &b) {
        cslibs_math::statistics::Distribution<3, 3> d;
        for (std::size_t i = 0 ; i < 8 ; ++i)
            d += b.at(i)->data();
        if (d.getN() == 0)
            return;

        cslibs_math_3d::Point3d mean(d.getMean());
        tmp.emplace_back(static_cast<float>(mean(0)));
        tmp.emplace_back(static_cast<float>(mean(1)));
        tmp.emplace_back(static_cast<float>(mean(2)));
        tmp.emplace_back(static_cast<float>(sample_bundle(b, mean)));
    };
    src.traverse(process_bundle);
    from(tmp, dst);
}

inline void from(
        const cslibs_ndt_3d::dynamic_maps::Gridmap::Ptr &src,
        sensor_msgs::PointCloud2 &dst)
{
    if (!src)
        return;

    from(*src, dst);
}

template<typename ndt_t,
        typename = typename std::enable_if<std::is_same<ndt_t, cslibs_ndt_3d::dynamic_maps::OccupancyGridmap>::value
                                           || std::is_same<ndt_t, cslibs_ndt_3d::static_maps::OccupancyGridmap>::value>::type>
inline void from(
        ndt_t &src,
        sensor_msgs::PointCloud2 &dst,
        const cslibs_gridmaps::utility::InverseModel::Ptr &ivm,
        const double &threshold = 0.169)
{
    src.allocatePartiallyAllocatedBundles();

    using index_t = std::array<int, 3>;
    using point_t = cslibs_math_3d::Point3d;
    using distribution_t = typename ndt_t::distribution_t;
    using distribution_bundle_t = typename ndt_t::distribution_bundle_t;
    auto sample = [&ivm](const distribution_t *d,
                         const point_t &p) -> double {
        auto evaluate = [&ivm, d, p] {
            const auto &handle = d;
            return d && handle->getDistribution() ?
                        handle->getDistribution()->sampleNonNormalized(p) * handle->getOccupancy(ivm) : 0.0;
        };
        return d ? evaluate() : 0.0;
    };
    auto sample_bundle = [&sample](const distribution_bundle_t &b,
                                   const point_t &p) -> double {
        return 0.125 * (sample(b.at(0), p) +
                        sample(b.at(1), p) +
                        sample(b.at(2), p) +
                        sample(b.at(3), p) +
                        sample(b.at(4), p) +
                        sample(b.at(5), p) +
                        sample(b.at(6), p) +
                        sample(b.at(7), p));
    };

    std::vector<float> tmp;
    auto process_bundle = [&src, &tmp, &ivm, &threshold, &sample_bundle](const index_t &bi, const distribution_bundle_t &b) {
        cslibs_math::statistics::Distribution<3, 3> d;
        double occupancy = 0.0;

        for (std::size_t i = 0 ; i < 8 ; ++i) {
            const auto &handle = b.at(i);
            occupancy += 0.125 * handle->getOccupancy(ivm);
            if (const auto &d_tmp = handle->getDistribution())
                d += *d_tmp;
        }
        if (d.getN() == 0 || occupancy < threshold)
            return;

        cslibs_math_3d::Point3d mean(d.getMean());
        tmp.emplace_back(static_cast<float>(mean(0)));
        tmp.emplace_back(static_cast<float>(mean(1)));
        tmp.emplace_back(static_cast<float>(mean(2)));
        tmp.emplace_back(static_cast<float>(sample_bundle(b, mean)));
    };
    src.traverse(process_bundle);
    from(tmp, dst);
}

inline void from(
        const cslibs_ndt_3d::dynamic_maps::OccupancyGridmap::Ptr &src,
        sensor_msgs::PointCloud2 &dst,
        const cslibs_gridmaps::utility::InverseModel::Ptr &ivm,
        const double &threshold = 0.169)
{
    if (!src)
        return;

    from(*src, dst, ivm, threshold);
}

}
}

#endif // CSLIBS_NDT_3D_CONVERSION_SENSOR_MSGS_POINTCLOUD2_HPP
