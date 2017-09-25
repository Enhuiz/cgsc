#include "cgsc/solver/solver.h"

#include <list>
#include <string>
#include <memory>

using namespace std;

namespace cgsc
{
namespace solver
{
Solver::Solver(const shared_ptr<Data> &data)
    : data(data)
{
}

list<Result> Solver::calculateResults()
{
    list<Result> results;
    for (const auto &aoi : data->getAOIs())
    {
        results.push_back(query(aoi));
    }
    return results;
}
}
}
