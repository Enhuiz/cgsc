#ifndef CGSC_SOLVER_SOLVER_HPP
#define CGSC_SOLVER_SOLVER_HPP

#include <list>
#include <string>
#include <memory>

#include "csv.hpp"
#include "data.hpp"

namespace cgsc
{
namespace solver
{
class Solver
{
  public:
    Solver(shared_ptr<Data> data)
        : data(data)
    {
    }

    virtual std::list<std::shared_ptr<const model::Scene>> query(std::shared_ptr<AOI> aoi) const = 0;

  private:
    Data data;
};
}
}

#endif