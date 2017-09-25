#include <string>
#include <list>
#include <list>
#include <memory>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include "cgsc/model/polygon.h"

using namespace std;

namespace cgsc
{
namespace model
{

Polygon::Polygon(const BoostPolygon &boostPolygon)
    : boostPolygon(boostPolygon)
{
}

Polygon::Polygon(const list<Point> &vertices)
{
    boost::geometry::append(boostPolygon, vertices);
}

Polygon::Polygon(const string &s)
    : Polygon(parseListOf<Point>(s))
{
}

list<Point> Polygon::outer() const
{
    auto boostOuter = boostPolygon.outer();
    return list<Point>(boostOuter.begin(), boostOuter.end());
}

bool Polygon::contains(const Point &point) const
{
    return boost::geometry::within(point, boostPolygon);
}

bool Polygon::overlaps(const shared_ptr<Polygon> &other) const
{
    return boost::geometry::overlaps(boostPolygon, other->boostPolygon);
}

bool Polygon::intersects(const shared_ptr<Polygon> &other) const
{
    return boost::geometry::intersects(boostPolygon, other->boostPolygon);
}

double Polygon::getArea() const
{
    return boost::geometry::area(boostPolygon);
}

string Polygon::to_string() const
{
    string ret;
    ostringstream oss(ret);

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

// friends

list<shared_ptr<Polygon>> union2(const shared_ptr<Polygon> &a, const shared_ptr<Polygon> &b)
{
    list<BoostPolygon> boostPolygons;
    boost::geometry::union_(a->boostPolygon, b->boostPolygon, boostPolygons);
    list<shared_ptr<Polygon>> ret;
    for (const auto &boostPolygon : boostPolygons)
    {
        ret.push_back(make_shared<Polygon>(boostPolygon));
    }
    return ret;
}

list<shared_ptr<Polygon>> union_(const list<shared_ptr<Polygon>> &polygons)
{
    auto performUnion = [](list<shared_ptr<Polygon>> &&polygons) {
        // input list will keep shrinking until finish

        const auto unionIteration = [&]() {
            for (auto i = polygons.begin(); i != polygons.end(); ++i)
            {
                for (auto j = next(i); j != polygons.end(); ++j)
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

        return list<shared_ptr<Polygon>>(polygons.begin(), polygons.end());
    };

    return performUnion(list<shared_ptr<Polygon>>(polygons.begin(), polygons.end()));
}

list<shared_ptr<Polygon>> intersection2(const shared_ptr<Polygon> &a, const shared_ptr<Polygon> &b)
{
    list<BoostPolygon> boostPolygons;
    boost::geometry::intersection(a->boostPolygon, b->boostPolygon, boostPolygons);
    list<shared_ptr<Polygon>> ret;
    for (const auto &boostPolygon : boostPolygons)
    {
        ret.push_back(make_shared<Polygon>(boostPolygon));
    }
    return ret;
}
}
}
