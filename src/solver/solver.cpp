#include "cgsc/solver/solver.h"

#include <list>
#include <string>
#include <memory>


namespace cgsc
{
namespace solver
{
Solver::Solver(const std::shared_ptr<Data> &data)
    : data(data)
{
}

std::list<Result> Solver::calculateResults()
{
    std::list<Result> ret;
    for (const auto &aoi : data->getAOIs())
    {
        ret.push_back(query(aoi));
    }
    return ret;
}
}
}
