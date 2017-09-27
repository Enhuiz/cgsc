#include <algorithm>
#include <memory>
#include <vector>
#include <limits>
#include <list>

#include "cgsc/solver/greedy.h"
#include "cgsc/model/grid.h"
#include "cgsc/utils/result.h"

using namespace std;
using namespace cgsc::model;
using namespace cgsc::utils;

namespace cgsc
{
namespace solver
{

vector<shared_ptr<const Scene>> Greedy::optimize(const AOI &aoi,
                                                 const vector<shared_ptr<const Scene>> &possibleScenes) const
{
    // only get scenes contains some grids in the AOI
    // here a list is used because we are gonna pick elements one by one from it
    list<shared_ptr<const Scene>> gridCoveringScenes;

    for (const auto &scene : possibleScenes)
    {
        // here, copy the scene so that we are free to modify them (e.g. add grids to it)
        auto newScene = make_shared<Scene>(*scene);

        newScene->setGrids(aoi.getGrids());
        if (newScene->getGrids().size() > 0)
        {
            // after modify the scene, make it const again
            gridCoveringScenes.push_back(shared_ptr<const Scene>(newScene));
        }
    }

    vector<shared_ptr<const Scene>> resultScenes;

    { // calculate result scenes
        set<shared_ptr<const Grid>> U;

        for (auto scene = pickGreedily(U, gridCoveringScenes);
             U.size() != aoi.getGrids().size() && scene != nullptr;
             scene = pickGreedily(U, gridCoveringScenes))
        {
            resultScenes.push_back(scene);

            for (const auto &grid : scene->getGrids())
            {
                U.insert(grid);
            }
        }
    }

    return resultScenes;
}

shared_ptr<const Scene> Greedy::pickGreedily(const set<shared_ptr<const Grid>> &U,
                                             list<shared_ptr<const Scene>> &gridCoveringScenes) const
{
    if (gridCoveringScenes.size() == 0)
    {
        return nullptr;
    }

    double minGamma = numeric_limits<double>::max();

    auto targetIt = gridCoveringScenes.begin();

    for (auto it = gridCoveringScenes.begin(); it != gridCoveringScenes.end(); ++it)
    {
        auto scene = *it;
        double currentGamma = gamma(scene->getPrice(), U, scene->getGrids());
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
    gridCoveringScenes.erase(targetIt); // erase the picked pointer, so that we won't pick it again

    return ret;
}

double Greedy::gamma(double price,
                     const set<shared_ptr<const Grid>> &U,
                     const set<shared_ptr<const Grid>> &S) const
{
    set<shared_ptr<const Grid>> diff;

    set_difference(S.begin(),
                   S.end(),
                   U.begin(),
                   U.end(),
                   inserter(diff, diff.begin()));

    if (diff.size() == 0)
    {
        // in case that the denominator is not zero
        return numeric_limits<double>::max();
    }

    return price / diff.size();
}
}
}
