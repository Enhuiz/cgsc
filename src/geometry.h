#ifndef CGSC_GEOMETRY_H
#define CGSC_GEOMETRY_H

#include <vector>
#include <set>
#include <string>

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

struct AOI
{
  std::string s;
  Polygon poly;
  CellSet cell_set;
};

struct Scene
{
  std::string s;
  Polygon poly;
  CellSet cell_set;
  double price;
};

struct Discretizer
{
  double delta;
  CellSet discretize(const Polygon &poly, bool keep_edge_point) const;
  CID point_to_cid(const Point &p) const;
  Point cid_to_point(const CID &cid) const;
  CID index_to_cid(int xi, int yi) const;
};


#endif