#ifndef CGSC_SOLVER_RESULT_HPP
#define CGSC_SOLVER_RESULT_HPP

#include <list>
#include <string>

#include "../model/scene.hpp"
#include "../model/aoi.hpp"
#include "../model/polygon.hpp"

namespace cgsc
{
namespace solver
{
class Result
{
  public:
    Result(std::shared_ptr<model::AOI> aoi, const std::list<std::shared_ptr<model::Scene>> &scenes)
        : aoi(aoi), scenes(scenes)
    {
        price = 0;
        for (const auto &scene : scenes)
        {
            price += scene->getPrice();
        }

        aoiArea = aoi->getArea();

        auto unionedPolygon = union_(std::list<std::shared_ptr<model::Polygon>>(scenes.begin(), scenes.end()));

        for (const auto &polygon : unionedPolygon)
        {
            auto intersectionPolygons = intersection2(aoi, polygon);
            for (const auto &intersectionPolygon : intersectionPolygons)
            {
                coverageArea += intersectionPolygon->getArea();
            }
        }
    }

    double getCoverageArea()
    {
        return coverageArea;
    }

    double getAOIArea()
    {
        return aoiArea;
    }

    double getCoverageRatio()
    {
        return coverageArea / aoiArea;
    }

  private:
    std::list<std::shared_ptr<model::Scene>> scenes;
    std::shared_ptr<model::AOI> aoi;

    double price;
    double aoiArea;
    double coverageArea;
};
}
}

#endif