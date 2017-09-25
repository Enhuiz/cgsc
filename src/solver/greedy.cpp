#include <algorithm>
#include <memory>

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

    { // get scenes only contains some grids in the AOI
        auto i = possibleScenes.begin();

        while (i != possibleScenes.end())
        {
            const auto &scene = *i;

            scene->setGrids(aoi->getGrids());

            if (scene->getGridCount() == 0)
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
    { //
        auto scene = pickScene(U, possibleScenes);
        resultScenes.push_back(scene);
        for (const auto &grid : scene->getGrids())
        {
            U.insert(grid);
        }
    }

    return Result(aoi, resultScenes);
}

shared_ptr<Scene> Greedy::pickScene(const set<shared_ptr<Grid>> &U,
                                    list<shared_ptr<Scene>> &possibleScenes) const
{
    double maxGamma = -1;

    auto targetIt = possibleScenes.begin();

    for (auto it = possibleScenes.begin(); it != possibleScenes.end(); ++it)
    {
        double currentGamma = gamma(U, **it);
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

double Greedy::gamma(const set<shared_ptr<Grid>> &U, const Scene &scene) const
{
    set<shared_ptr<Grid>> common;

    set_intersection(U.begin(), U.end(),
                     scene.getGrids().begin(), scene.getGrids().end(),
                     inserter(common, common.begin()));

    return scene.getPrice() / common.size();
}
}
}
