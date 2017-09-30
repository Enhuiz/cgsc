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

void Result::addPossibleScenes(const vector<shared_ptr<const Scene>> &scenes, bool verbose)
{
    jobj["possibleScenes"] = {};
    for (const auto &scene : scenes)
    {
        jobj["possibleScenes"].push_back(scene->toJSON(verbose));
    }
}

void Result::addTotalPrice(const vector<shared_ptr<const Scene>> &scenes)
{
    double totalPrice = 0;
    for (const auto &scene : scenes)
    {
        totalPrice += scene->getPrice();
    }
    jobj["totalPrice"] = totalPrice;
}

double Result::calculateCoverageArea(const AOI &aoi, const vector<shared_ptr<const Scene>> &scenes) const
{
    double coverageArea = 0;

    auto unionedPolygon = union_(list<shared_ptr<const Polygon>>(scenes.begin(), scenes.end()));
    for (const auto &polygon : unionedPolygon)
    {
        auto intersectionPolygons = intersection2(aoi, *polygon);
        for (const auto &intersectionPolygon : intersectionPolygons)
        {
            coverageArea += intersectionPolygon->getArea();
        }
    }
    return coverageArea;
}

void Result::addCoverageArea(const AOI &aoi, const vector<shared_ptr<const Scene>> &scenes)
{
    jobj["coverageArea"] = calculateCoverageArea(aoi, scenes);
}

void Result::addCoverageRatio(const AOI &aoi, const vector<shared_ptr<const Scene>> &scenes)
{
    double coverageArea = calculateCoverageArea(aoi, scenes);
    jobj["coverageRatio"] = coverageArea / aoi.getArea();
}

void Result::addResultScense(const vector<shared_ptr<const Scene>> &scenes, bool verbose)
{
    jobj["resultScenes"] = {};
    for (const auto &scene : scenes)
    {
        jobj["resultScenes"].push_back(scene->toJSON(verbose));
    }
}

void Result::addAOI(const AOI &aoi, bool verbose)
{
    jobj["aoi"] = aoi.toJSON(verbose);
}

void Result::addJSON(const string &tag, const nlohmann::json &j)
{
    jobj[tag] = j;
}

nlohmann::json Result::toJSON() const
{
    return jobj;
}

ostream &operator<<(ostream &os, const Result &result)
{
    os << result.jobj;
}
}
}