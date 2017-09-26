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
  Greedy(std::shared_ptr<Data> data);

  Result query(const model::AOI &aoi) const;

private:
  std::shared_ptr<model::Scene> pickGreedily(const std::set<std::shared_ptr<model::Grid>> &U,
                                             std::list<std::shared_ptr<model::Scene>> &possibleScenes) const;

  double gamma(double price, const std::set<std::shared_ptr<model::Grid>> &U, const std::set<std::shared_ptr<model::Grid>> &S) const;
};
}
}

#endif