#include "cgsc/model/grid.h"

#include <iostream>

using namespace std;

namespace cgsc
{
namespace model
{
Grid::Grid(int xi, int yi, double delta)
    : xi(xi), yi(yi), delta(delta),
      Polygon({Point(xi * delta, yi * delta),
               Point(xi * delta + delta, yi * delta),
               Point(xi * delta + delta, yi * delta + delta),
               Point(xi * delta, yi * delta + delta)})
{
}

bool Grid::operator==(const Grid &other) const
{
    return xi == other.xi && yi == other.yi && delta == other.delta;
}

bool Grid::operator!=(const Grid &other) const
{
    return xi != other.xi || yi != other.yi || delta != other.delta;
}

bool Grid::operator<(const Grid &other) const
{
    return *this != other;
}

bool operator==(const std::shared_ptr<Grid> &a, const std::shared_ptr<Grid> &b)
{
    return *a == *b;
}

bool operator!=(const std::shared_ptr<Grid> &a, const std::shared_ptr<Grid> &b)
{
    return *a != *b;
}

bool operator<(const std::shared_ptr<Grid> &a, const std::shared_ptr<Grid> &b)
{
    return *a < *b;
}

bool operator==(const std::shared_ptr<const Grid> &a, const std::shared_ptr<const Grid> &b)
{
    return *a == *b;
}

bool operator!=(const std::shared_ptr<const Grid> &a, const std::shared_ptr<const Grid> &b)
{
    return *a != *b;
}

bool operator<(const std::shared_ptr<const Grid> &a, const std::shared_ptr<const Grid> &b)
{
    return *a < *b;
}




}
}