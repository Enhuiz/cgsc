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
}

const std::set<std::shared_ptr<const Grid>> &AOI::getGrids() const
{
    return grids;
}

void AOI::updateGrids()
{
    const auto &vertices = outer();

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

    int minxi = floor(minx / delta);
    int minyi = floor(miny / delta);

    int maxxi = ceil(maxx / delta);
    int maxyi = ceil(maxy / delta);

    grids.clear();
    for (int i = minxi; i < maxxi; ++i)
    {
        for (int j = minyi; j < maxyi; ++j)
        {
            auto grid = make_shared<Grid>(i, j, delta);
            if (intersects(*grid))
            {
                grids.insert(grid);
            }
        }
    }
}

nlohmann::json AOI::toJSON(bool verbose) const
{
    auto jobj = Polygon::toJSON();

    jobj["delta"] = delta;
    jobj["area"] = getArea();

    if (verbose)
    {
        jobj["grids"] = {};
        for (const auto &grid : grids)
        {
            jobj["grids"].push_back(grid->toJSON());
        }
    }

    return jobj;
}
}
}
