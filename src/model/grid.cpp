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
    // this one is killing me!
    // return *this != other;

    // if you are gonna use set_xxxxxxx function, make sure your container is ordered!

    if (xi != other.xi)
    {
        return xi < other.xi;
    }
    else if (yi != other.yi)
    {
        return yi < other.yi;
    }
    else
    {
        return delta < other.delta;
    }
}

ostream &operator<<(ostream &os, const Grid &grid)
{
    os << grid.xi << ", " << grid.yi << ", " << grid.delta;
    return os;
}

// bool operator==(const shared_ptr<Grid> &a, const shared_ptr<Grid> &b)
// {
//     return *a == *b;
// }

// bool operator!=(const shared_ptr<Grid> &a, const shared_ptr<Grid> &b)
// {
//     return *a != *b;
// }

bool operator<(const shared_ptr<Grid> &a, const shared_ptr<Grid> &b)
{
    return *a < *b;
}

// bool operator==(const shared_ptr<const Grid> &a, const shared_ptr<const Grid> &b)
// {
//     return *a == *b;
// }

// bool operator!=(const shared_ptr<const Grid> &a, const shared_ptr<const Grid> &b)
// {
//     return *a != *b;
// }

bool operator<(const shared_ptr<const Grid> &a, const shared_ptr<const Grid> &b)
{
    return *a < *b;
}
}
}