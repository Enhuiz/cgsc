#ifndef CGSC_SOLVER_GREEDY_HPP
#define CGSC_SOLVER_GREEDY_HPP

#include <algorithm>
#include <memory>

#include "../model/grid.hpp"

#include "solver.hpp"

#include "result.hpp"

namespace cgsc
{
namespace solver
{
class Greedy : public Solver
{
  public:
    Greedy(const std::shared_ptr<Data> &data)
        : Solver(data)
    {
    }

    Result query(const std::shared_ptr<model::AOI>& aoi) const
    {
        std::list<std::shared_ptr<model::Scene>> possibleScenes;

        for (const auto &scene : data->getScenes())
        {
            if (aoi->overlaps(scene))
            {
                // here, copy the scene so that we are free modify them (e.g. add grids to it)
                possibleScenes.push_back(std::make_shared<model::Scene>(*scene));
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

        std::list<std::shared_ptr<model::Scene>> resultScenes;

        std::set<std::shared_ptr<model::Grid>> U;
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

  private:
    std::shared_ptr<model::Scene> pickScene(const std::set<std::shared_ptr<model::Grid>> &U,
                                     std::list<std::shared_ptr<model::Scene>> &possibleScenes) const
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

    double gamma(const std::set<std::shared_ptr<model::Grid>> &U, const model::Scene &scene) const
    {
        std::set<std::shared_ptr<model::Grid>> common;

        std::set_intersection(U.begin(), U.end(),
                              scene.getGrids().begin(), scene.getGrids().end(),
                              std::inserter(common, common.begin()));

        return scene.getPrice() / common.size();
    }
};
}
}

#endif