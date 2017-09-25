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

#include "cgsc/model/utils.h"

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

  std::list<Point> outer() const;

  bool contains(const Point &point) const;

  bool overlaps(const std::shared_ptr<Polygon> &other) const;

  bool intersects(const std::shared_ptr<Polygon> &other) const;

  double getArea() const;

  std::string to_string() const;

public:
  friend std::list<std::shared_ptr<Polygon>> union_(const std::list<std::shared_ptr<Polygon>> &polygons);
  friend std::list<std::shared_ptr<Polygon>> union2(const std::shared_ptr<Polygon> &a, const std::shared_ptr<Polygon> &b);

  friend std::list<std::shared_ptr<Polygon>> intersection2(const std::shared_ptr<Polygon> &a, const std::shared_ptr<Polygon> &b);

protected:
  BoostPolygon boostPolygon;
};
}
}

#endif