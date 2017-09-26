#pragma once
#ifndef CGSC_MODEL_AOI_H
#define CGSC_MODEL_AOI_H

#include <string>
#include <list>
#include <memory>
#include <vector>

#include "cgsc/model/polygon.h"
#include "cgsc/model/grid.h"

namespace cgsc
{
namespace model
{
class AOI : public Polygon
{
public:
  AOI(const std::list<Point> &vertices, double delta = 1);

  AOI(const std::string &s, double delta = 1);

  void setDelta(double delta);

  nlohmann::json toJSON() const;

  const std::set<std::shared_ptr<const Grid>> &getGrids() const;

private:
  void updateGrids();

private:
  std::set<std::shared_ptr<const Grid>> grids;

  double delta;
};
}
}

#endif