#ifndef CGSC_GEOMETRY_H
#define CGSC_GEOMETRY_H

#include <list>
#include <string>
#include <functional>

struct Vector2
{
  double x;
  double y;
  Vector2 operator+(const Vector2 &other) const;
  Vector2 operator-(const Vector2 &other) const;
  bool operator==(const Vector2 &other) const;
  bool operator!=(const Vector2 &other) const;
  bool almost_equal(const Vector2 &other, int ulp) const;
};

double cross(const Vector2 &a, const Vector2 &b);

using Point = Vector2;
using Polygon = std::list<Point>;
using Triangle = Polygon;

std::string to_string(const Vector2 &v);
std::string to_string(const Polygon &poly);
std::ostream &operator<<(std::ostream &os, const Vector2 &v);
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
// Following functions only support non-closed polygon representation (i.e. first point != last point)
std::list<Triangle> triangulate(const Polygon &poly); // to support concave
// Following functions only support convex clipper
std::tuple<std::list<Polygon>, std::list<Polygon>> clip(const Polygon &clippee, const Polygon &clipper);
std::list<Polygon> intersection(const Polygon &clippee, const Polygon &clipper);
std::list<Polygon> intersection(std::list<Polygon> clippees, const std::list<Polygon> &clipper);
std::list<Polygon> difference(const Polygon &clippee, const Polygon &clipper);
std::list<Polygon> difference(std::list<Polygon> clippees, const std::list<Polygon> &clipper);

#endif