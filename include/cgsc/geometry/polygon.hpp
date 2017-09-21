#ifndef CGSC_MODEL_POLYGON_HPP
#define CGSC_MODEL_POLYGON_HPP

#include <string>
#include <vector>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include "utils.hpp"

namespace cgsc
{
namespace model
{

class Polygon
{
  public:
    Polygon(const std::vector<Point> &vertices)
    {
        boost::geometry::append(polygon, vertices);
    }

    Polygon(const std::string &s) : Polygon(parsePoints(s))
    {
    }

    bool contains(const Point &point) const
    {
        return boost::geometry::within(point, polygon);
    }

    bool overlaps(const Polygon &other) const
    {
        return boost::geometry::overlaps(polygon, other.polygon);
    }

    bool intersects(const Polygon &other) const
    {
        return boost::geometry::intersects(polygon, other.polygon);
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

  protected:
    boost::geometry::model::polygon<Point, false, false> polygon;
};
}
}

#endif