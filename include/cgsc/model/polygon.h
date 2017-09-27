#pragma once
#ifndef CGSC_MODEL_POLYGON_H
#define CGSC_MODEL_POLYGON_H

#include <string>
#include <list>
#include <list>
#include <memory>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include "json.hpp"
#include "cgsc/model/misc.h"

namespace cgsc
{
namespace model
{

using BoostPolygon = boost::geometry::model::polygon<Point, false, false>;

class Polygon
{
public:
  Polygon(const BoostPolygon &boostPolygon);

  Polygon(const std::list<Point> &vertices);

  Polygon(const std::string &s);

  std::vector<Point> outer() const;

  bool contains(const Point &point) const;

  bool overlaps(const Polygon &other) const;

  bool intersects(const Polygon &other) const;

  double getArea() const;

  std::string toString() const;

  virtual nlohmann::json toJSON() const;

public:
  friend std::list<std::shared_ptr<const Polygon>> union_(const std::list<std::shared_ptr<const Polygon>> &polygons);
  friend std::list<std::shared_ptr<const Polygon>> union2(const Polygon &a, const Polygon &b);

  friend std::list<std::shared_ptr<const Polygon>> intersection2(const Polygon &a, const Polygon &b);

private:
  double area;
  BoostPolygon boostPolygon;
};
}
}

#endif