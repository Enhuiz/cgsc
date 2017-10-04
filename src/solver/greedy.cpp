#include <algorithm>
#include <memory>
#include <vector>
#include <limits>
#include <list>

#include "cgsc/solver/greedy.h"
#include "cgsc/model/cell.h"
#include "cgsc/utils/result.h"
#include "cgsc/utils/timestamp.h"

using namespace std;
using namespace cgsc::model;
using namespace cgsc::utils;

namespace cgsc
{
namespace solver
{

vector<shared_ptr<const Scene>> Greedy::optimize(const AOI &aoi,
                                                 const vector<shared_ptr<const Scene>> &cellCoveringScenes) const
{
    auto resultScenes = vector<shared_ptr<const Scene>>();

    auto clonedCellCoveringScenes = list<shared_ptr<const Scene>>(cellCoveringScenes.begin(),
                                                                  cellCoveringScenes.end());


    { // calculate result scenes
        auto U = set<CellID>();

        for (auto scene = pickGreedily(U, clonedCellCoveringScenes);
             U.size() != aoi.getCells().size() && scene != nullptr;
             scene = pickGreedily(U, clonedCellCoveringScenes))
        {
            resultScenes.push_back(scene);

            for (const auto &grid : scene->getCells())
            {
                U.insert(grid);
            }
        }
    }

    return resultScenes;
}

shared_ptr<const Scene> Greedy::pickGreedily(const set<CellID> &U,
                                             list<shared_ptr<const Scene>> &cellCoveringScenes) const
{
    if (cellCoveringScenes.size() == 0)
    {
        return nullptr;
    }

    double minGamma = numeric_limits<double>::max();

    auto targetIt = cellCoveringScenes.begin();

    for (auto it = cellCoveringScenes.begin(); it != cellCoveringScenes.end(); ++it)
    {
        auto scene = *it;
        double currentGamma = gamma(scene->getPrice(), U, scene->getCells());
        if (minGamma > currentGamma)
        {
            targetIt = it;
            minGamma = currentGamma;
        }
    }

    if (minGamma == numeric_limits<double>::max()) // no way to improve
    {
        return nullptr;
    }

    auto ret = *targetIt;               // save our target pointer
    cellCoveringScenes.erase(targetIt); // erase the picked pointer, so that we won't pick it again

    return ret;
}

double Greedy::gamma(double price,
                     const set<CellID> &U,
                     const set<CellID> &S) const
{
    auto diff = set<CellID>();

    set_difference(S.begin(),
                   S.end(),
                   U.begin(),
                   U.end(),
                   inserter(diff, diff.begin())); // this is not compared by value but pointer, notice

    if (diff.size() == 0)
    {
        // in case that the denominator is not zero
        return numeric_limits<double>::max();
    }

    return price / diff.size();
}
}
}
