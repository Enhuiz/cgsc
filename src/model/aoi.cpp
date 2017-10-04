#include "cgsc/model/aoi.h"

using namespace std;

namespace cgsc
{
namespace model
{

AOI::AOI(const list<Point> &vertices)
    : Polygon(vertices)
{
}

AOI::AOI(const string &s)
    : AOI(parseListOf<Point>(s))
{
}

const std::set<CellID> &AOI::getCells() const
{
    return cells;
}

void AOI::updateCells()
{
    cells = Cell::GetCellIDSetByPolygon(this, true);
}

nlohmann::json AOI::toJSON(bool verbose) const
{
    auto jobj = Polygon::toJSON();

    jobj["area"] = getArea();

    if (verbose)
    {
        jobj["cells"] = {};
        for (const auto &cell : cells)
        {
            auto polygon = Cell::GetPolygonByCellID(cell);
            jobj["cells"].push_back(polygon.toJSON());
        }
    }

    return jobj;
}
}
}
