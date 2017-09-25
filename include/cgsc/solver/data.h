#pragma once  
#ifndef CGSC_SOLVER_DATA_H
#define CGSC_SOLVER_DATA_H

#include <vector>
#include <string>
#include <memory>

#include "cgsc/model/scene.h"
#include "cgsc/model/aoi.h"


namespace cgsc
{
namespace solver
{
class Data
{
  public:
    Data(const std::string &scenesPath, const std::string &aoiPath);

    std::vector<std::shared_ptr<model::Scene>> getScenes() const;
    std::vector<std::shared_ptr<model::AOI>> getAOIs() const;

  private:
    void loadScenes(const std::string &path, int maxRecords = -1);
    void loadAOIs(const std::string &path, int maxRecords = -1);

  private:
    std::vector<std::shared_ptr<model::Scene>> scenes;
    std::vector<std::shared_ptr<model::AOI>> aois;
};
}
}

#endif