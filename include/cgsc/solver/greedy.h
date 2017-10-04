#pragma once
#ifndef CGSC_SOLVER_GREEDY_H
#define CGSC_SOLVER_GREEDY_H

#include <algorithm>
#include <memory>

#include "cgsc/model/cell.h"
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
                                                            const std::vector<std::shared_ptr<const model::Scene>> &cellCoveringScenes) const;

private:
  std::shared_ptr<const model::Scene> pickGreedily(const std::set<model::CellID> &U,
                                                   std::list<std::shared_ptr<const model::Scene>> &gridCoveringScenes) const;

  double gamma(double price,
               const std::set<model::CellID> &U,
               const std::set<model::CellID> &S) const;
};
}
}

#endif