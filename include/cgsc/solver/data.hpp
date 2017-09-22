#ifndef CGSC_SOLVER_DATA_HPP
#define CGSC_SOLVER_DATA_HPP

#include <vector>
#include <string>
#include <memory>

#include "csv.hpp"
#include "../model/scene.hpp"
#include "../model/aoi.hpp"

namespace cgsc
{
namespace solver
{
class Data
{
  public:
    Data(const std::string &scenesPath, const std::string &aoiPath)
    {
        loadScenes(scenesPath);
        loadAOIs(aoiPath);
    }

    std::vector<std::shared_ptr<model::Scene>> getScenes() const
    {
        return scenes;
    }

    std::vector<std::shared_ptr<model::AOI>> getAOIs() const
    {
        return aois;
    }
 
  private:
    void loadScenes(const std::string &path, int maxRecords = -1)
    {
        io::CSVReader<2, io::trim_chars<' '>, io::double_quote_escape<',', '\"'>> in(path);
        in.read_header(io::ignore_extra_column, "Polygon", "Price");
        std::string polygon;
        double price;
        while (in.read_row(polygon, price) && maxRecords != 0)
        {
            scenes.push_back(std::make_shared<model::Scene>(polygon, price));
            --maxRecords;
        }
        std::cout << scenes.size() << " scenes loaded" << std::endl;
    }

    void loadAOIs(const std::string &path, int maxRecords = -1)
    {
        io::CSVReader<1, io::trim_chars<' '>, io::double_quote_escape<',', '\"'>> in(path);
        in.read_header(io::ignore_extra_column, "Polygon");
        std::string polygon;
        while (in.read_row(polygon) && maxRecords != 0)
        {
            aois.push_back(std::make_shared<model::AOI>(polygon));
            --maxRecords;
        }
        std::cout << aois.size() << " AOIs loaded" << std::endl;
    }

  private:
    std::vector<std::shared_ptr<model::Scene>> scenes;
    std::vector<std::shared_ptr<model::AOI>> aois;
};
}
}

#endif