#pragma once
#ifndef CGSC_MODEL_CELL_H
#define CGSC_MODEL_CELL_H

#include "cgsc/model/polygon.h"

#include <limits>
#include <memory>
#include <iostream>
#include <utility>

namespace cgsc
{
namespace model
{
using CellID = unsigned long long;

class Cell
{
public:
  static void SetDelta(double delta);

  static std::set<CellID> GetCellIDSetByPolygon(Polygon *polygon, bool aggresive);
  static Polygon GetPolygonByCellID(CellID);
  
private:
  static CellID IndexToCellID(int xi, int yi);
  static bool Inside(int xi, int yi, Polygon *polygon);
  static bool Intersect(int xi, int yi, Polygon *polygon);
  static Polygon GetPolygonByIndex(int xi, int yi);
private:
  Cell();
  static double delta;
};
}
}

#endif