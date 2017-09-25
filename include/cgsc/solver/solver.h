#pragma once  
#ifndef CGSC_SOLVER_SOLVER_H
#define CGSC_SOLVER_SOLVER_H

#include <list>
#include <string>
#include <memory>

#include "cgsc/model/aoi.h"

#include "cgsc/solver/result.h"
#include "cgsc/solver/data.h"

namespace cgsc
{
namespace solver
{
class Solver
{
  public:
    Solver(const std::shared_ptr<Data> &data);

    virtual Result query(const std::shared_ptr<model::AOI> &aoi) const = 0;

    std::list<Result> calculateResults();

  protected:
    std::shared_ptr<Data> data;
};
}
}

#endif