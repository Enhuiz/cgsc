#ifndef CGSC_MODEL_POLYGON_HPP
#define CGSC_MODEL_POLYGON_HPP

#include <string>
#include <vector>
#include <list>
#include <memory>

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

    std::vector<Point> outer() const
    {
        return boostPolygon.outer();
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

  public:
    friend std::vector<std::shared_ptr<Polygon>> union_(const std::vector<std::shared_ptr<Polygon>> &polygons);
    friend std::vector<std::shared_ptr<Polygon>> union2(const Polygon &a, const Polygon &b);

    // friend std::vector<std::shared_ptr<Polygon>> intersection(const std::vector<std::shared_ptr<Polygon>> &polygons);
    friend std::vector<std::shared_ptr<Polygon>> intersection2(const Polygon &a, const Polygon &b);

  protected:
    BoostPolygon boostPolygon;
};

std::vector<std::shared_ptr<Polygon>> union2(const Polygon &a, const Polygon &b)
{
    std::vector<BoostPolygon> boostPolygons;
    boost::geometry::union_(a.boostPolygon, b.boostPolygon, boostPolygons);
    std::vector<std::shared_ptr<Polygon>> ret;
    for (const auto &boostPolygon : boostPolygons)
    {
        ret.push_back(std::make_shared<Polygon>(boostPolygon));
    }
    return ret;
}

std::vector<std::shared_ptr<Polygon>> union_(const std::vector<std::shared_ptr<Polygon>> &polygons)
{
    auto performUnion = [](std::list<std::shared_ptr<Polygon>> &&polygons) {
        // input list will keep shrinking until finish

        const auto unionIteration = [&]() {
            for (auto i = polygons.begin(); i != polygons.end(); ++i)
            {
                for (auto j = std::next(i); j != polygons.end(); ++j)
                {
                    const auto &polygon1 = **i;
                    const auto &polygon2 = **j;

                    if (polygon1.overlaps(polygon2)) // only try two merge two polygon when they overlaps
                    {
                        auto unionedPolygons = union2Polygons(polygon1, polygon2);
                        for (const auto &unionedPolygon : unionedPolygons)
                        {
                            polygons.push_back(unionedPolygon);
                        }
                        return true;
                    }
                }
            }
            return false;
        };

        while (unionIteration())
            ;

        return std::vector<std::shared_ptr<Polygon>>(polygons.begin(), polygons.end());
    };

    return performUnion(std::list<std::shared_ptr<Polygon>>(polygons.begin(), polygons.end()));
}

std::vector<std::shared_ptr<Polygon>> intersection2(const Polygon &a, const Polygon &b)
{
    std::vector<BoostPolygon> boostPolygons;
    boost::geometry::intersection(a.boostPolygon, b.boostPolygon, boostPolygons);
    std::vector<std::shared_ptr<Polygon>> ret;
    for (const auto &boostPolygon : boostPolygons)
    {
        ret.push_back(std::make_shared<Polygon>(boostPolygon));
    }
    return ret;
}
}
}

#endif