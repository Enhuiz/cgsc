#pragma once
#ifndef CGSC_MODEL_SCENE_H
#define CGSC_MODEL_SCENE_H

#include <string>
#include <list>
#include <set>

#include "cgsc/model/polygon.h"
#include "cgsc/model/grid.h"

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

  void setGrids(const std::set<std::shared_ptr<const Grid>> &grids);


  const std::set<std::shared_ptr<const Grid>>& getGrids() const;

  nlohmann::json toJSON() const;

private:
  bool covers(const Grid &grid) const;

private:
  std::set<std::shared_ptr<const Grid>> grids;
  
  double price;
};
}
}

#endif