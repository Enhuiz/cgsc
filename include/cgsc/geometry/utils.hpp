#ifndef CGSC_MODEL_UTILS_HPP
#define CGSC_MODEL_UTILS_HPP

#include <string>
#include <vector>
#include <sstream>
#include <cctype>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>

namespace cgsc
{
namespace model
{

using Point = boost::geometry::model::d2::point_xy<double>;

std::vector<Point> parsePoints(std::string s)
{
    // required string format:
    // [[0.105, 0.105], [0.2, 0.2], ..., [0.2, 0.2]]

    std::vector<double> nums;

    const auto isValidChar = [](char ch) {
        return std::isdigit(ch) || ch == '.' || ch == '-';
    };

    const auto nextl = [&](int l) {
        while (l < s.size() && !isValidChar(s[l]))
            ++l;
        return l;
    };

    int l = nextl(0);
    int r = l + 1;

    while (r < s.size())
    {
        if (!isValidChar(s[r]))
        {
            nums.push_back(std::stod(s.substr(l, r - l)));
            r = l = nextl(r);
        }
        ++r;
    }

    if (l < s.size())
    {
        nums.push_back(std::stod(s.substr(l, r - l)));
    }

    std::vector<Point> points;
    for (int i = 0; i < nums.size(); i += 2)
    {
        double x = nums[i];
        double y = nums[i + 1];
        points.push_back(Point(x, y));
    }

    return points;
}
}
}

#endif