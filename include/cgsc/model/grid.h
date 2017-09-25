#pragma once
#ifndef CGSC_MODEL_GRID_H
#define CGSC_MODEL_GRID_H

#include "cgsc/model/polygon.h"

#include <memory>

namespace cgsc
{
namespace model
{
class Grid : public Polygon
{
public:
  Grid(int x, int y, double delta);

  bool operator==(const Grid &other) const;
  bool operator!=(const Grid &other) const;
  bool operator<(const Grid &other) const;

public:
  friend bool operator==(const std::shared_ptr<Grid> &a, const std::shared_ptr<Grid> &b);
  friend bool operator!=(const std::shared_ptr<Grid> &a, const std::shared_ptr<Grid> &b);
  friend bool operator<(const std::shared_ptr<Grid> &a, const std::shared_ptr<Grid> &b);

private:
  int x, y;
  double delta;
};
}
}

#endif