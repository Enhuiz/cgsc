#include "cgsc/solver/solver.h"

#include "cgsc/utils/timestamp.h"

#include <vector>
#include <string>
#include <memory>

using namespace std;
using namespace cgsc::utils;
using namespace cgsc::model;

namespace cgsc
{
namespace solver
{

// possible scenes is the scenes which intersects the AOI
vector<shared_ptr<const Scene>> Solver::selectPossibleScenes(const AOI &aoi,
                                                     const vector<shared_ptr<const Scene>> &scenes) const
{
    vector<shared_ptr<const Scene>> ret;
    for (const auto &scene : scenes)
    {
        if (aoi.intersects(*scene))
        {
            ret.push_back(scene);
        }
    }
    return ret;
}


}
}
