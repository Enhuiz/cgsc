#ifndef CGSC_GEOMETRY_H
#define CGSC_GEOMETRY_H

#include <list>
#include <string>
#include <functional>

#include "vector2.hpp"

using Point = vector2<double>;
using Polygon = std::list<Point>;
using Triangle = Polygon;
using Polygons = std::list<Polygon>;

std::string to_string(const Polygon &poly);
std::ostream &operator<<(std::ostream &os, const Polygon &poly);

std::list<Point> parse_polygon(const std::string &s);
double area(const Polygon &poly);
bool inside(const Point &p, const Point &a, const Point &b);
bool inside(const Point &p, const Polygon &poly); // only for convex
bool outside(const Point &p, const Point &a, const Point &b);
bool outside(const Point &p, const Polygon &poly); // only for convex
bool crosses(const Polygon &a, const Polygon &b);
bool disjoint(const Polygon &a, const Polygon &b);
bool intersects(const Polygon &a, const Polygon &b);
bool convex(const Polygon &poly);

Polygon box(const Point &lower_left, const Point &upper_right);
Polygon axis_aligned_bounding_box(const Polygon &polygon);

// Following clip related functions only support non-closed polygon representation (i.e. first point != last point)
// and convex clippers
std::tuple<Polygons, Polygons> clip(const Polygon &clippee, const Polygon &clipper);
Polygons intersection(const Polygon &clippee, const Polygon &clipper);
Polygons intersection(Polygons clippees, const Polygons &clipper);
Polygons difference(const Polygon &clippee, const Polygon &clipper);
Polygons difference(Polygons clippees, const Polygons &clipper);

#endif