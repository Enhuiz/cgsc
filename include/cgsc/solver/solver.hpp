#ifndef CGSC_SOLVER_SOLVER_HPP
#define CGSC_SOLVER_SOLVER_HPP

#include <vector>
#include <string>
#include <memory>

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

    virtual std::vector<std::shared_ptr<const model::Scene>> query(std::shared_ptr<AOI> aoi) const = 0;

  private:
    std::vector<std::shared_ptr<model::Scene>> scenes;
};
}
}

#endif