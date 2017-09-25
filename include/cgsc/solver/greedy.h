#pragma once  
#ifndef CGSC_SOLVER_GREEDY_H
#define CGSC_SOLVER_GREEDY_H

#include <algorithm>
#include <memory>

#include "cgsc/model/grid.h"

#include "solver.h"
#include "result.h"

namespace cgsc
{
namespace solver
{
class Greedy : public Solver
{
  public:
    Greedy(const std::shared_ptr<Data> &data);

    Result query(const std::shared_ptr<model::AOI> &aoi) const;

  private:
    std::shared_ptr<model::Scene> pickScene(const std::set<std::shared_ptr<model::Grid>> &U,
                                            std::list<std::shared_ptr<model::Scene>> &possibleScenes) const;

    double gamma(const std::set<std::shared_ptr<model::Grid>> &U, const model::Scene &scene) const;
};
}
}

#endif