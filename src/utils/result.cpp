#include <list>
#include <string>
#include <fstream>

#include "cgsc/utils/result.h"

#include "cgsc/model/scene.h"
#include "cgsc/model/aoi.h"
#include "cgsc/model/polygon.h"

using namespace std;
using namespace cgsc::model;

namespace cgsc
{
namespace solver
{
Result::Result(const AOI &aoi, const list<shared_ptr<Scene>> &scenes)
{
    double price = 0;
    for (const auto &scene : scenes)
    {
        price += scene->getPrice();
    }

    jobj["price"] = price;
    jobj["aoi"] = aoi.toJSON();

    jobj["scenes"] = {};
    for (const auto &scene : scenes)
    {
        jobj["scenes"].push_back(scene->toJSON());
    }

    double coverageArea = 0;

    auto unionedPolygon = union_(list<shared_ptr<Polygon>>(scenes.begin(), scenes.end()));
    for (const auto &polygon : unionedPolygon)
    {
        auto intersectionPolygons = intersection2(aoi, *polygon);
        for (const auto &intersectionPolygon : intersectionPolygons)
        {
            coverageArea += intersectionPolygon->getArea();
        }
    }

    jobj["coverage_area"] = coverageArea;
    jobj["coverage_ratio"] = coverageArea / aoi.getArea();
}

void Result::save(const string &path) const
{
    ofstream ofs(path);
    ofs << jobj << endl;
}

ostream &operator<<(ostream &os, const Result &result)
{
    os << result.jobj << endl;
}
}
}