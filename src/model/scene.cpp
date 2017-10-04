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

const set<CellID> &Scene::getCells() const
{
    return cells;
}

void Scene::updateCells()
{
    cells = Cell::GetCellIDSetByPolygon(this, false);
}

void Scene::filterCells(set<CellID> aoiCells)
{
    auto oldCells = cells;
    cells.clear();
    set_intersection(oldCells.begin(),
                     oldCells.end(),
                     aoiCells.begin(),
                     aoiCells.end(),
                     inserter(cells, cells.begin()));
}

nlohmann::json Scene::toJSON(bool verbose) const
{
    auto jobj = Polygon::toJSON();
    if (verbose)
    {
        jobj["cells"] = {};
        for (const auto &cell : cells)
        {
            auto polygon = Cell::GetPolygonByCellID(cell);
            jobj["cells"].push_back(polygon.toJSON());
        }
    }
    jobj["area"] = getArea();
    jobj["price"] = price;
    return jobj;
}
}
}