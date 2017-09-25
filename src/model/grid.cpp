#include "cgsc/model/grid.h"

namespace cgsc
{
namespace model
{
Grid::Grid(int x, int y, double delta)
    : x(x), y(y), delta(delta),
      Polygon({Point(x * delta, y * delta),
               Point(x * delta + delta, y * delta),
               Point(x * delta + delta, y * delta + delta),
               Point(x * delta, y * delta + delta)})
{
}

bool Grid::operator==(const Grid &other) const
{
    return x == other.x && y == other.y && delta == other.delta;
}

bool Grid::operator!=(const Grid &other) const
{
    return x != other.x || y != other.y || delta != other.delta;
}
}
}