#pragma once  
#ifndef CGSC_MODEL_GRID_H
#define CGSC_MODEL_GRID_H

#include "cgsc/model/polygon.h"

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

  private:
    int x, y;
    double delta;
};
}
}

#endif