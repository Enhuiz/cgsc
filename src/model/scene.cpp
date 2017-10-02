#include <string>
#include <list>
#include <set>

#include "cgsc/model/scene.h"

using namespace std;

namespace cgsc
{
namespace model
{

Scene::Scene(const list<Point> &vertices, double price)
    : Polygon(vertices), price(price)
{
}

Scene::Scene(const string &s, double price)
    : Scene(parseListOf<Point>(s), price)
{
}

double Scene::getPrice() const
{
    return price;
}

const ConstGridPtrSet &Scene::getGrids() const
{
    return grids;
}

bool Scene::covers(const Grid &grid) const
{
    for (const Point &point : grid.outer())
    {
        if (!contains(point))
        {
            return false;
        }
    }
    return true;
}

void Scene::updateGrids(const AOI &aoi) // cost less memory, maybe time costing
{
    grids.clear();
    for (const auto &grid : aoi.getGrids())
    {
        if (covers(*grid))
        {
            grids.insert(grid);
        }
    }
}

void Scene::updateGrids(double delta, const AOI &aoi) // memory costing, maybe faster
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

    int minxi = ceil(minx / delta); // ceil for min
    int minyi = ceil(miny / delta);

    int maxxi = floor(maxx / delta); // floor for max
    int maxyi = floor(maxy / delta);

    grids.clear();
    const auto &aoiGrids = aoi.getGrids();
    for (int i = minxi; i < maxxi; ++i)
    {
        for (int j = minyi; j < maxyi; ++j)
        {
            auto grid = make_shared<const Grid>(i, j, delta); // create a new grid, which is memory consuming
            if (covers(*grid))    // covering is neccessary, and should be in aoi
            {
                grids.insert(grid);
            }
        }
    }

    // filter
    auto sceneGrids = grids;
    grids.clear();

    set_intersection(sceneGrids.begin(),
                     sceneGrids.end(),
                     aoiGrids.begin(),
                     aoiGrids.end(),
                     inserter(grids, grids.begin()));
}

nlohmann::json Scene::toJSON(bool verbose) const
{
    auto jobj = Polygon::toJSON();
    if (verbose)
    {
        jobj["grids"] = {};
        for (const auto &grid : grids)
        {
            jobj["grids"].push_back(grid->toJSON());
        }
    }
    jobj["area"] = getArea();
    jobj["price"] = price;
    return jobj;
}
}
}