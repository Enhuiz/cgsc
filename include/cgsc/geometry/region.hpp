#ifndef CGSC_MODEL_REGION_HPP
#define CGSC_MODEL_REGION_HPP

#include <string>
#include <vector>
#include <sstream>
#include <cctype>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

namespace cgsc
{
namespace model
{
using Point = boost::geometry::model::d2::point_xy<double>;
using Polygon = boost::geometry::model::polygon<Point, false>;

std::vector<Point> parsePoints(std::string s)
{
    // required string format:
    // [[0.105, 0.105], [0.2, 0.2], ..., [0.2, 0.2]]

    std::vector<double> nums;

    auto isdigitdot = [](char ch) {
        return std::isdigit(ch) || ch == '.';
    };

    auto nextl = [&](int l) {
        while (l < s.size() && !isdigitdot(s[l]))
            ++l;
        return l;
    };

    int l = nextl(0);
    int r = l + 1;
    
    while (r < s.size())
    {
        if (!isdigitdot(s[r]))
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

class Region
{
  public:
    Region(const std::vector<Point> &vertices, double price = 0) : price(price)
    {
        boost::geometry::append(polygon, vertices);
        area = boost::geometry::area(polygon);
    }

    Region(const std::string &s) : Region(parsePoints(s))
    {
    }

    bool within(const Point &point) const
    {
        // return boost::geometry::within(polygon, point);
        return true;
    }

    double getArea() const
    {
        return area;
    }

    double getPrice() const
    {
        return price;
    }

    std::string to_string() const
    {
        std::string ret;
        std::ostringstream oss(ret);

        oss << "[";
        const auto &vertices = polygon.outer();
        for (int i = 0; i < vertices.size(); ++i)
        {
            if (i)
            {
                oss << ", ";
            }
            oss << "[" << vertices[i].x() << ", " << vertices[i].y() << "]";
        }
        oss << "]";

        return oss.str();
    }

    std::ostream &operator<<(std::ostream &os) const
    {
        os << to_string();
    }

  private:
    Polygon polygon;
    double price;
    double area;
};
}
}

#endif