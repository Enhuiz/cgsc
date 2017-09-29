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
  Grid(int xi, int yi, double delta);

  bool operator==(const Grid &other) const;
  bool operator!=(const Grid &other) const;
  bool operator<(const Grid &other) const;

public:
  friend bool operator==(const std::shared_ptr<Grid> &a, const std::shared_ptr<Grid> &b);
  friend bool operator!=(const std::shared_ptr<Grid> &a, const std::shared_ptr<Grid> &b);
  friend bool operator<(const std::shared_ptr<Grid> &a, const std::shared_ptr<Grid> &b);

  friend bool operator==(const std::shared_ptr<const Grid> &a, const std::shared_ptr<const Grid> &b);
  friend bool operator!=(const std::shared_ptr<const Grid> &a, const std::shared_ptr<const Grid> &b);
  friend bool operator<(const std::shared_ptr<const Grid> &a, const std::shared_ptr<const Grid> &b);


private:
  int xi, yi;
  double delta;
};
}
}

#endif