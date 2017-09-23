#ifndef CGSC_MODEL_POLYGON_HPP
#define CGSC_MODEL_POLYGON_HPP

#include <string>
#include <list>
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

    Polygon(const std::list<Point> &vertices)
    {
        boost::geometry::append(boostPolygon, vertices);
    }

    Polygon(const std::string &s)
        : Polygon(parseListOf<Point>(s))
    {
    }

    std::list<Point> outer() const
    {
        auto boostOuter = boostPolygon.outer();
        return std::list<Point>(boostOuter.begin(), boostOuter.end());
    }

    bool contains(const Point &point) const
    {
        return boost::geometry::within(point, boostPolygon);
    }

    bool overlaps(const std::shared_ptr<Polygon> &other) const
    {
        return boost::geometry::overlaps(boostPolygon, other->boostPolygon);
    }

    bool intersects(const std::shared_ptr<Polygon> &other) const
    {
        return boost::geometry::intersects(boostPolygon, other->boostPolygon);
    }

    double getArea() const
    {
        return boost::geometry::area(boostPolygon);
    }

    std::string to_string() const
    {
        std::string ret;
        std::ostringstream oss(ret);

        const auto &vertices = boostPolygon.outer();

        oss << "[";
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
    friend std::list<std::shared_ptr<Polygon>> union_(const std::list<std::shared_ptr<Polygon>> &polygons);
    friend std::list<std::shared_ptr<Polygon>> union2(const std::shared_ptr<Polygon> &a, const std::shared_ptr<Polygon> &b);

    friend std::list<std::shared_ptr<Polygon>> intersection2(const std::shared_ptr<Polygon> &a, const std::shared_ptr<Polygon> &b);

  protected:
    BoostPolygon boostPolygon;
};

std::list<std::shared_ptr<Polygon>> union2(const std::shared_ptr<Polygon> &a, const std::shared_ptr<Polygon> &b)
{
    std::list<BoostPolygon> boostPolygons;
    boost::geometry::union_(a->boostPolygon, b->boostPolygon, boostPolygons);
    std::list<std::shared_ptr<Polygon>> ret;
    for (const auto &boostPolygon : boostPolygons)
    {
        ret.push_back(std::make_shared<Polygon>(boostPolygon));
    }
    return ret;
}

std::list<std::shared_ptr<Polygon>> union_(const std::list<std::shared_ptr<Polygon>> &polygons)
{
    auto performUnion = [](std::list<std::shared_ptr<Polygon>> &&polygons) {
        // input list will keep shrinking until finish

        const auto unionIteration = [&]() {
            for (auto i = polygons.begin(); i != polygons.end(); ++i)
            {
                for (auto j = std::next(i); j != polygons.end(); ++j)
                {
                    const auto &polygon1 = *i;
                    const auto &polygon2 = *j;

                    if (polygon1->overlaps(polygon2)) // only try two merge two polygon when they overlaps
                    {
                        auto unionedPolygons = union2(polygon1, polygon2);
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

        return std::list<std::shared_ptr<Polygon>>(polygons.begin(), polygons.end());
    };

    return performUnion(std::list<std::shared_ptr<Polygon>>(polygons.begin(), polygons.end()));
}

std::list<std::shared_ptr<Polygon>> intersection2(const std::shared_ptr<Polygon> &a, const std::shared_ptr<Polygon> &b)
{
    std::list<BoostPolygon> boostPolygons;
    boost::geometry::intersection(a->boostPolygon, b->boostPolygon, boostPolygons);
    std::list<std::shared_ptr<Polygon>> ret;
    for (const auto &boostPolygon : boostPolygons)
    {
        ret.push_back(std::make_shared<Polygon>(boostPolygon));
    }
    return ret;
}
}
}

#endif