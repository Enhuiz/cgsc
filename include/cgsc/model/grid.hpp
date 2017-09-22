#ifndef CGSC_MODEL_GRID_HPP
#define CGSC_MODEL_GRID_HPP

#include <string>

#include <boost/geometry.hpp>

#include "polygon.hpp"

namespace cgsc
{
namespace model
{
class Grid : public Polygon
{
  public:
    Grid(int x, int y, double delta)
        : x(x), y(y), delta(delta),
          Polygon({Point(x * delta, y * delta),
                   Point(x * delta + delta, y * delta),
                   Point(x * delta + delta, y * delta + delta),
                   Point(x * delta, y * delta + delta)})
    {
    }

    bool operator==(const Grid &other) const
    {
        return x == other.x && y == other.y && delta == other.delta;
    }

    bool operator!=(const Grid &other) const
    {
        return x != other.x || y != other.y || delta != other.delta;
    }

  private:
    int x, y;
    double delta;
};
}
}

#endif