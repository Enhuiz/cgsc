#pragma once
#ifndef CGSC_MODEL_AOI_H
#define CGSC_MODEL_AOI_H

#include <string>
#include <list>
#include <memory>
#include <vector>

#include "cgsc/model/polygon.h"
#include "cgsc/model/cell.h"

namespace cgsc
{
namespace model
{
class AOI : public Polygon
{
public:
  AOI(const std::list<Point> &vertices);
  AOI(const std::string &s);

  nlohmann::json toJSON(bool verbose) const;

  const std::set<CellID> &getCells() const;

  void updateCells();

private:
  std::set<CellID> cells;
};
}
}

#endif