#include <list>
#include <string>
#include <fstream>

#include "cgsc/utils/result.h"

#include "cgsc/model/scene.h"
#include "cgsc/model/aoi.h"
#include "cgsc/model/polygon.h"

#include "cgsc/utils/timestamp.h"

using namespace std;
using namespace cgsc::model;

namespace cgsc
{
namespace utils
{
Result::Result(const AOI &aoi,
               const vector<shared_ptr<const Scene>> &possibleScenes,
               const vector<shared_ptr<const Scene>> &resultScenes)
{
    double price = 0;
    for (const auto &scene : resultScenes)
    {
        price += scene->getPrice();
    }
    jobj["price"] = price;

    jobj["aoi"] = aoi.toJSON();

    {
        auto addScenes = [&](const std::string &tag,
                            const vector<shared_ptr<const Scene>> &scenes) {
            jobj[tag] = {};
            for (const auto &scene : scenes)
            {
                jobj[tag].push_back(scene->toJSON());
            }
        };

        addScenes("possible_scenes", possibleScenes);
        addScenes("result_scenes", resultScenes);
    }

    double coverageArea = 0;

    auto unionedPolygon = union_(list<shared_ptr<const Polygon>>(resultScenes.begin(), resultScenes.end()));
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
    os << result.jobj;
}
}
}