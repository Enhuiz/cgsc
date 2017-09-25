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

double Result::getCoverageArea() const
{
    return coverageArea;
}

double Result::getAOIgetArea() const
{
    return aoi->getArea();
}

double Result::getCoverageRatio() const
{
    return coverageArea / aoi->getArea();
}

shared_ptr<const AOI> Result::getAOI() const
{
    return aoi;
}

double Result::getPrice() const 
{
    return price;
}

void Result::save(const string& path) 
{
    
}

ostream &operator<<(ostream &os, const Result &result)
{
    os << "AOI: " << result.aoi->toString() << endl
       << "Number of feasible scenes: " << result.scenes.size() << endl
       << "AOI area: " << result.aoi->getArea() << endl
       << "Coverage area: " << result.getCoverageArea() << endl
       << "Coverage ratio: " << result.getCoverageRatio() << endl       
       << "Price: " << result.getPrice() << endl;       
    return os;
}


}
}