#pragma once
#ifndef CGSC_SOLVER_GREEDY_H
#define CGSC_SOLVER_GREEDY_H

#include <algorithm>
#include <memory>

#include "cgsc/model/grid.h"
#include "cgsc/solver/solver.h"
#include "cgsc/utils/result.h"

namespace cgsc
{
namespace solver
{
class Greedy : public Solver
{
public:
  std::vector<std::shared_ptr<const model::Scene>> optimize(const model::AOI &aoi,
                                                            const std::vector<std::shared_ptr<const model::Scene>> &possibleScenes) const;

private:
  std::shared_ptr<const model::Scene> pickGreedily(const std::set<std::shared_ptr<const model::Grid>> &U,
                                                   std::list<std::shared_ptr<const model::Scene>> &gridCoveringScenes) const;

  double gamma(double price, const std::set<std::shared_ptr<const model::Grid>> &U,
               const std::list<std::shared_ptr<const model::Grid>> &S) const;
};
}
}

#endif