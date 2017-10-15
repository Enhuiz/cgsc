#ifndef CGSC_GEOMETRY_H
#define CGSC_GEOMETRY_H

#include <vector>
#include <set>
#include <string>
#include <functional>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

struct Vector2
{
  double x;
  double y;
  Vector2 operator+(const Vector2 &other) const;
  Vector2 operator-(const Vector2 &other) const;
};

double cross_product(const Vector2 &a, const Vector2 &b);

using Point = Vector2;

using Polygon = std::vector<Point>;

std::vector<Point> parse_polygon(const std::string &poly_s);
bool contains(const Polygon &poly, const Point &p);
bool contains_all(const Polygon &a, const std::vector<Point> &ps);
bool contains_any(const Polygon &a, const std::vector<Point> &ps);

bool crosses(const Polygon &a, const Polygon &b);
bool disjoint(const Polygon &a, const Polygon &b);
bool intersects(const Polygon &a, const Polygon &b);
double area(const Polygon &poly);
std::string to_string(const Polygon &poly);

using CID = unsigned long long;

using CellSet = std::set<CID>;

using BoostPoint = boost::geometry::model::d2::point_xy<float>;
using BoostPolygon = boost::geometry::model::polygon<BoostPoint, false, false>;

struct AOI
{
  std::string s;
  Polygon poly;
  CellSet cell_set;                 // discrete
  std::vector<BoostPolygon> bpolys; // continous
};

struct Scene
{
  std::string s;
  Polygon poly;
  double price;
  CellSet cell_set;
  std::vector<BoostPolygon> bpolys;
};

struct Discretizer
{
  double delta;
  CellSet discretize(const Polygon &poly, bool keep_edge_cells) const;
  CID point_to_cid(const Point &p) const;
  Point cid_to_point(const CID &cid) const;
  CID index_to_cid(int xi, int yi) const;
  std::tuple<double, double, double, double> axis_aligned_bounding_box_lu_corner(const Polygon &poly, const std::function<double(double)> &min_trunc, const std::function<double(double)> &max_trunc) const;
  Polygon axis_aligned_bounding_box(const Polygon &poly, const std::function<double(double)> &min_trunc, const std::function<double(double)> &max_trunc) const;
};

#endif