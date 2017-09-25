#pragma once  
#ifndef CGSC_SOLVER_RESULT_H
#define CGSC_SOLVER_RESULT_H

#include <list>
#include <string>

#include "cgsc/model/scene.h"
#include "cgsc/model/aoi.h"
#include "cgsc/model/polygon.h"

namespace cgsc
{
namespace solver
{
class Result
{
  public:
    Result(std::shared_ptr<model::AOI> aoi, const std::list<std::shared_ptr<model::Scene>> &scenes);

    double getCoverageArea();

    double getAOIArea();

    double getCoverageRatio();

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