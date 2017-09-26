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
  Solver(std::shared_ptr<utils::Data> data);

  virtual utils::Result query(const model::AOI &aoi) const = 0;

  std::vector<utils::Result> calculateResults();

protected:
  std::shared_ptr<utils::Data> data;
};
}
}

#endif