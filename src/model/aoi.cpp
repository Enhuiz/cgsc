#include "cgsc/model/aoi.h"

using namespace std;

namespace cgsc
{
namespace model
{

AOI::AOI(const list<Point> &vertices, double delta)
    : Polygon(vertices)
{
    setDelta(delta);
}

AOI::AOI(const string &s, double delta)
    : AOI(parseListOf<Point>(s), delta)
{
}

void AOI::setDelta(double delta)
{
    this->delta = delta;

    this->updateGrids();
}

set<shared_ptr<Grid>> AOI::getGrids() const
{
    return grids;
}

void AOI::updateGrids()
{
    const auto &vertices = outer();

    double minx, miny, maxx, maxy;

    for (const auto &vertex : vertices)
    {
        minx = min(minx, vertex.x());
        miny = min(miny, vertex.y());
        maxx = max(maxx, vertex.x());
        maxy = max(maxy, vertex.y());
    }

    int minxi = floor(minx / delta);
    int minyi = floor(miny / delta);

    int maxxi = ceil(maxx / delta);
    int maxyi = ceil(maxy / delta);

    for (int i = minxi; i < maxxi; ++i)
    {
        for (int j = minyi; j < maxyi; ++j)
        {
            auto grid = make_shared<Grid>(i, j, delta);
            if (overlaps(grid))
            {
                grids.insert(grid);
            }
        }
    }
}
}
}
