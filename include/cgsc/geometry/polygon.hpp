#ifndef CGSC_MODEL_POLYGON_HPP
#define CGSC_MODEL_POLYGON_HPP

#include <string>
#include <vector>
#include <list>

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
    friend std::vector<Polygon> unionPolygons(const std::vector<Polygon> polygons);
    friend std::vector<Polygon> unionPolygons(const std::list<Polygon> polygons);

  protected:
    BoostPolygon<Point, false, false> boostPolygon;
};

friend std::vector<Polygon> unionPolygons(const std::vector<Polygon> polygons)
{
    return unionPolygons(std::list(polygons.begin(), polygons.end()));
}

friend std::vector<Polygon> unionPolygons(const std::list<Polygon> polygons)
{
    const auto union2Polygons = [](const Polygon &a, const Polygon &b) {
        std::vector<BoostPolygon> boostPolygons;
        boost::geometry::union_(a, b, boostPolygons);
        std::vector<Polygon> ret;
        for (const auto &boostPolygon : boostPolygons)
        {
            ret.push_back(Polygon(boostPolygon));
        }
        return ret;
    };

    const auto unionIteration = [&]() {
        for (auto i = polygons.begin(); i != polygons.end(); ++i)
        {
            for (auto j = i + 1; j != polygons.end(); ++j)
            {
                auto unionedPolygons = union2Polygons(*i, *j);
                for (const auto &poly : unionedPolygons)
                {
                    polygons.push_back(poly);
                }
                return polygons.size();
            }
        }
        return polygons.size();
    }

    while (polygons.size() != unionIteration());

    return std::vector<Polygon>(polygons.begin(), polygons.end());
}
}
}

#endif