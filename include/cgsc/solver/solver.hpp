#ifndef CGSC_SOLVER_SOLVER_HPP
#define CGSC_SOLVER_SOLVER_HPP

#include <vector>
#include <string>

#include "csv.hpp"
#include "../geometry/scene.hpp"
#include "../geometry/aoi.hpp"

namespace cgsc
{
namespace solver
{
class Solver
{
  public:
    Solver(std::string path, int maxRecords = -1)
    {
        io::CSVReader<3> in(path);
        in.read_header(io::ignore_extra_column, "Polygon", "Price");
        std::string polygon;
        double price;
        while (in.read_row(polygon, price) && maxRecords > 0)
        {
            scenes.emplace_back(polygon, price);
            --maxRecords;
        }
        std::cout << scenes.size() << " records loaded" << std::endl;
    }

    std::vector<model::Scene> getScenes(model::AOI aoi)
    {
        std::vector<model::Scene> overlappedScenes;
        for ()
    }

  private:
    std::vector<model::Scene> scenes;
};
}
}

#endif