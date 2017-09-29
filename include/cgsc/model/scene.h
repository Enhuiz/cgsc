#pragma once
#ifndef CGSC_MODEL_SCENE_H
#define CGSC_MODEL_SCENE_H

#include <string>
#include <list>

#include "cgsc/model/polygon.h"
#include "cgsc/model/grid.h"
#include "cgsc/model/aoi.h"

namespace cgsc
{
namespace model
{

class Scene : public Polygon
{
public:
  Scene(const std::list<Point> &vertices, double price = 0);

  Scene(const std::string &s, double price);

  double getPrice() const;

  void updateGrids(double delta);

  void filterGrids(const AOI& aoi);

  const std::list<std::shared_ptr<const Grid>> &getGrids() const;

  nlohmann::json toJSON(bool verbose) const;

private:
  bool covers(const Grid &grid) const;

private:
  std::list<std::shared_ptr<const Grid>> grids;

  double price;
};
}
}

#endif