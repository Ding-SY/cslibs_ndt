/// PROJECT
#include "../../data/pointcloud.hpp"
#include "../../ndt/matcher2D.hpp"
#include "visualize.hpp"

void linspace(const double min,
              const double max,
              const double res,
              std::vector<double> &values)
{
    const double        range = max - min;
    const std::size_t   intervals = range / res;
    for(std::size_t i = 0 ; i < intervals ; ++i) {
        values.push_back(min + res * i);
    }
}

typedef ndt::NDTMatcher2D           Matcher;

int main(int argc, char *argv[])
{
    std::vector<Matcher::PointType> points_src;
    /// generate horizontal lines
    std::vector<double> xs;
    linspace(-10.0, -1.0, 0.1, xs);
    for(double &e : xs) {
        points_src.push_back(Matcher::PointType(e, 1.0));
        points_src.push_back(Matcher::PointType(e, -1.0));
    }
    /// generate vertial lines
    std::vector<double> ys;
    linspace(-10.0, 10.0, 0.1, ys);
    for(double &e : ys) {
        points_src.push_back(Matcher::PointType(1.5, e));
        if(e < -1.0 || e > 1.0)
            points_src.push_back(Matcher::PointType(-1.0, e));
    }
    /// generate a second points test which is transformed
    Matcher::RotationType    rotation       = Matcher::RotationType(0.0);
    Matcher::TranslationType trans          = Matcher::TranslationType(0.2, 0.0);
    Matcher::TransformType   transformation = trans * rotation;
    std::vector<Matcher::PointType> points_dst;
    for(Matcher::PointType &p : points_src) {
        points_dst.push_back(transformation * p);
    }

    ndt::NDTMultiGrid2D::Size   size = {20, 20};
    ndt::NDTMultiGrid2D::Resolution resolution = {1.0, 1.0};
    ndt::data::Pointcloud<2> pointcloud_src(points_src);
    ndt::data::Pointcloud<2> pointcloud_dst(points_dst);

    /// show the point set
    cv::Mat display = cv::Mat(800, 800, CV_8UC3, cv::Scalar());
    ndt::renderPoints(points_src, size, resolution, display, cv::Scalar(255), false, 0.5, false);
    ndt::renderPoints(points_dst, size, resolution, display, cv::Scalar(0,255), false, 0.5, true);
    while(true) {
        cv::imshow("display", display);
        int key = cv::waitKey(0) & 0xFF;
        if(key == 27)
            break;
    }
    cv::flip(display, display, 0);

    /// now we can try out the matching
    Matcher m(resolution);
    m.match(pointcloud_src, pointcloud_dst, transformation, 4000, 1e-3);

    for(Matcher::PointType &p : points_dst) {
        p = transformation * p;
    }
    ndt::renderPoints(points_dst, size, resolution, display, cv::Scalar(0,0,255), false, 0.5, true);

    while(true) {
        cv::imshow("display", display);
        int key = cv::waitKey(0) & 0xFF;
        if(key == 27)
            break;
    }

    return 0;
}