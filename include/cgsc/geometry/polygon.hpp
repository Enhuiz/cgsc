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

using BoostPolygon = boost::geometry::model::polygon<Point, false, false>;

class Polygon
{
  public:
    Polygon(const BoostPolygon &boostPolygon)
        : boostPolygon(boostPolygon)
    {
    }

    Polygon(const std::vector<Point> &vertices)
    {
        boost::geometry::append(boostPolygon, vertices);
    }

    Polygon(const std::string &s)
        : Polygon(parsePoints(s))
    {
    }

    bool contains(const Point &point) const
    {
        return boost::geometry::within(point, boostPolygon);
    }

    bool overlaps(const Polygon &other) const
    {
        return boost::geometry::overlaps(boostPolygon, other.boostPolygon);
    }

    bool intersects(const Polygon &other) const
    {
        return boost::geometry::intersects(boostPolygon, other.boostPolygon);
    }

    std::string to_string() const
    {
        std::string ret;
        std::ostringstream oss(ret);

        oss << "[";
        const auto &vertices = boostPolygon.outer();
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

  public:
    friend std::vector<Polygon> union(const std::vector<Polygon> polygons);
    
  protected:
    BoostPolygon<Point, false, false> boostPolygon;
};

friend std::vector<Polygon> union(const std::vector<Polygon> polygons)
{
    const auto union2Polygon = [](const Polygon &a, const Polygon &b) {
        std::vector<BoostPolygon> boostPolygons;
        boost::geometry::union_(a, b, boostPolygons);
        std::vector<Polygon> ret;
        for (const auto &boostPolygon : boostPolygons)
        {
            ret.push_back(Polygon(boostPolygon));
        }
        return ret;
    };

    const auto extractUnion = [](std::list<Polygon> leftPolygons) {
        auto polygon = leftPolygons.head();

    };

}
}
}

#endif