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

void Scene::setGrids(const set<shared_ptr<Grid>> &grids)
{
    for (const auto &grid : grids)
    {
        if (covers(*grid))
        {
            this->grids.insert(grid);
        }
    }
}

set<shared_ptr<Grid>> Scene::getGrids() const
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

nlohmann::json Scene::toJSON() const
{
    auto jobj = Polygon::toJSON();
    jobj["price"] = price;
    return jobj;
}
}
}