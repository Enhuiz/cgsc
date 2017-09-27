#pragma once
#ifndef CGSC_SOLVER_SOLVER_H
#define CGSC_SOLVER_SOLVER_H

#include <vector>
#include <string>
#include <memory>

#include "cgsc/model/aoi.h"

#include "cgsc/utils/result.h"
#include "cgsc/utils/data.h"

namespace cgsc
{
namespace solver
{
class Solver
{
public:
  std::vector<std::shared_ptr<const model::Scene>> optimize(const model::AOI &aoi,
                                                            const std::vector<std::shared_ptr<const model::Scene>> &possibleScenes) const;

  std::vector<std::shared_ptr<const model::Scene>> selectPossibleScenes(const model::AOI &aoi,
                                                                        const std::vector<std::shared_ptr<const model::Scene>> &scenes) const;
};
}
}

#endif