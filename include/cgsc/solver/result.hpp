#ifndef CGSC_SOLVER_RESULT_HPP
#define CGSC_SOLVER_RESULT_HPP

#include <vector>
#include <string>

#include "../geometry/scene.hpp"
#include "../geometry/aoi.hpp"

namespace cgsc
{
namespace solver
{
class Result
{
  public:
    Result(const model::AOI &aoi, const std::vector<model::Scene> &scenes)
        : aoi(aoi), scenes(scenes)
    {
        price = 0;
        for (const auto &scene : scenes)
        {
            price += scene.getPrice();
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
    std::vector<model::Scene> scenes;
    model::AOI aoi;

    double price;
    double aoiArea;
    double coverageArea;
};
}
}

#endif