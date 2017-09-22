#ifndef CGSC_SOLVER_GREEDY_HPP
#define CGSC_SOLVER_GREEDY_HPP

#include "solver.hpp"
#include "../geometry/"

namespace cgsc
{
namespace solver
{
class Greedy : public Solver
{
  public:
    std::list<std::shared_ptr<const model::Scene>> query(std::shared_ptr<model::AOI> aoi) const
    {
        std::list<std::shared_ptr<model::Scene>> possibleScenes;

        for (const auto &scene : scenes)
        {
            if (aoi.overlaps(*scene))
            {
                // here, copy the scene so that we are free modify them (e.g. add grids to it)
                possibleScenes.emplace_back(*scene);
            }
        }

        { // get scenes only contains some grids in the AOI
            auto i = possibleScenes.begin();

            while (i != possibleScenes.end())
            {
                const auto &scene = **i;
                scene.setGrids(aoi.getGrids());
                if (scene.getGridCount() == 0)
                {
                    possibleScenes.remove(i++);
                }
                else
                {
                    ++i;
                }
            }
        }

        std::set<model::Grid> 

        
    }
};
}
}

#endif