#pragma once
#ifndef CGSC_MODEL_SCENE_H
#define CGSC_MODEL_SCENE_H

#include <string>
#include <list>

#include "cgsc/model/polygon.h"
#include "cgsc/model/cell.h"
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

  void updateCells();
  void filterCells(std::set<CellID> aoiCells);

  const std::set<CellID> &getCells() const;

  nlohmann::json toJSON(bool verbose) const;

private:
  std::set<CellID> cells;

  double price;
};
}
}

#endif