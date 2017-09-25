#include <algorithm>
#include <memory>
#include <vector>
#include <limits>

#include "cgsc/solver/greedy.h"
#include "cgsc/model/grid.h"
#include "cgsc/solver/result.h"

using namespace std;
using namespace cgsc::model;

namespace cgsc
{
namespace solver
{
Greedy::Greedy(const shared_ptr<Data> &data)
    : Solver(data)
{
}

Result Greedy::query(const shared_ptr<AOI> &aoi) const
{
    list<shared_ptr<Scene>> possibleScenes;

    for (const auto &scene : data->getScenes())
    {
        if (aoi->overlaps(scene))
        {
            // here, copy the scene so that we are free modify them (e.g. add grids to it)
            possibleScenes.push_back(make_shared<Scene>(*scene));
        }
    }

    cerr << "number of possible scenes " << possibleScenes.size() << endl;

    { // get scenes only contains some grids in the AOI
        auto i = possibleScenes.begin();

        while (i != possibleScenes.end())
        {
            const auto &scene = *i;

            scene->setGrids(aoi->getGrids());
            if (scene->getGrids().size() == 0)
            {
                possibleScenes.erase(i++);
            }
            else
            {
                ++i;
            }
        }
    }

    list<shared_ptr<Scene>> resultScenes;
    set<shared_ptr<Grid>> U;

    while (U.size() != aoi->getGrids().size() && possibleScenes.size() > 0)
    {
        auto scene = pickGreedily(U, possibleScenes);
        resultScenes.push_back(scene);
        for (const auto &grid : scene->getGrids())
        {
            U.insert(grid);
        }
    }

    return Result(aoi, resultScenes);
}

shared_ptr<Scene> Greedy::pickGreedily(const set<shared_ptr<Grid>> &U,
                                       list<shared_ptr<Scene>> &possibleScenes) const
{
    double maxGamma = -1;

    auto targetIt = possibleScenes.begin();

    for (auto it = possibleScenes.begin(); it != possibleScenes.end(); ++it)
    {
        auto scene = *it;
        double currentGamma = gamma(scene->getPrice(), U, scene->getGrids());
        if (maxGamma < currentGamma)
        {
            targetIt = it;
            maxGamma = currentGamma;
        }
    }

    auto ret = *targetIt;
    possibleScenes.erase(targetIt);
    return ret;
}

double Greedy::gamma(double price, const set<shared_ptr<Grid>> &U, const set<shared_ptr<Grid>> &S) const
{
    set<shared_ptr<Grid>> diff;

    set_intersection(S.begin(),
                     S.end(),
                     U.begin(),
                     U.end(),
                     inserter(diff, diff.begin()));

    if (diff.size() == 0)
    {
        return numeric_limits<double>::max();
    }

    return price / diff.size();
}
}
}
