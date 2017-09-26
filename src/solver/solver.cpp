#include "cgsc/solver/solver.h"

#include "cgsc/utils/timestamp.h"

#include <vector>
#include <string>
#include <memory>

using namespace std;
using namespace cgsc::utils;

namespace cgsc
{
namespace solver
{
Solver::Solver(shared_ptr<Data> data)
    : data(data)
{
}

vector<Result> Solver::calculateResults()
{
    vector<Result> results;
    for (const auto &aoi : data->getAOIs())
    {
        Timestamp::Add("begin query");
        results.push_back(query(*aoi));
        Timestamp::Add("end query");
    }
    return results;
}

}
}
