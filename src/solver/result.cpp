#include <list>
#include <string>

#include "cgsc/solver/result.h"

#include "cgsc/model/scene.h"
#include "cgsc/model/aoi.h"
#include "cgsc/model/polygon.h"

using namespace std;
using namespace cgsc::model;

namespace cgsc
{
namespace solver
{
Result::Result(shared_ptr<AOI> aoi, const list<shared_ptr<Scene>> &scenes)
    : aoi(aoi), scenes(scenes)
{
    price = 0;
    for (const auto &scene : scenes)
    {
        price += scene->getPrice();
    }

    aoiArea = aoi->getArea();

    auto unionedPolygon = union_(list<shared_ptr<Polygon>>(scenes.begin(), scenes.end()));

    for (const auto &polygon : unionedPolygon)
    {
        auto intersectionPolygons = intersection2(aoi, polygon);
        for (const auto &intersectionPolygon : intersectionPolygons)
        {
            coverageArea += intersectionPolygon->getArea();
        }
    }
}

double Result::getCoverageArea()
{
    return coverageArea;
}

double Result::getAOIArea()
{
    return aoiArea;
}

double Result::getCoverageRatio()
{
    return coverageArea / aoiArea;
}
}
}