#include "cgsc/model/cell.h"

#include <iostream>

using namespace std;

namespace cgsc
{
namespace model
{
double Cell::delta = 0;

void Cell::SetDelta(double delta)
{
    Cell::delta = delta;
}

std::set<CellID> Cell::GetCellIDSetByPolygon(Polygon *polygon, bool aggresive)
{
    const auto &vertices = polygon->outer();

    double minx = vertices[0].x();
    double miny = vertices[0].y();
    double maxx = vertices[0].x();
    double maxy = vertices[0].y();

    for (const auto &vertex : vertices)
    {
        minx = min(minx, vertex.x());
        miny = min(miny, vertex.y());
        maxx = max(maxx, vertex.x());
        maxy = max(maxy, vertex.y());
    }

    int minxi, minyi, maxxi, maxyi;

    if (aggresive)
    {
        minxi = floor(minx / delta);
        minyi = floor(miny / delta);

        maxxi = ceil(maxx / delta);
        maxyi = ceil(maxy / delta);
    }
    else
    {
        minxi = ceil(minx / delta);
        minyi = ceil(miny / delta);

        maxxi = floor(maxx / delta);
        maxyi = floor(maxy / delta);
    }

    std::set<CellID> ret;

    auto cond = aggresive ? Intersect : Inside;

    for (int i = minxi; i < maxxi; ++i)
    {
        for (int j = minyi; j < maxyi; ++j)
        {
            if (cond(i, j, polygon)) // intersections is ok
            {
                ret.insert(IndexToCellID(i, j));
            }
        }
    }
    return ret;
}

CellID Cell::IndexToCellID(int xi, int yi)
{
    return xi + ((CellID)yi << 32);
}

Polygon Cell::GetPolygonByIndex(int xi, int yi)
{
    double x = xi * delta;
    double y = yi * delta;

    return Polygon({Point(x, y), Point(x + delta, y), Point(x + delta, y + delta), Point(x, y + delta)});
}

Polygon Cell::GetPolygonByCellID(CellID cid)
{
    return GetPolygonByIndex(cid & (unsigned int)(-1), cid >> 32);
}

bool Cell::Inside(int xi, int yi, Polygon *polygon)
{
    double x = xi * delta;
    double y = yi * delta;

    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            if (!polygon->contains(Point(x + i * delta, y + j * delta)))
            {
                return false;
            }
        }
    }
    return true;
}

bool Cell::Intersect(int xi, int yi, Polygon *polygon)
{
    return polygon->intersects(GetPolygonByIndex(xi, yi));
}
}
}
