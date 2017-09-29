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
namespace utils
{
class Data
{
public:
  Data(const std::string &scenesPath, const std::string &aoiPath);

  const std::vector<std::shared_ptr<const model::Scene>> &getScenes() const;
  const std::vector<std::shared_ptr<const model::AOI>> &getAOIs() const;

  std::vector<std::shared_ptr<model::Scene>> cloneScenes() const;
  std::vector<std::shared_ptr<model::AOI>> cloneAOIs() const;

private:
  void loadScenes(const std::string &path, int maxRecords = -1);
  void loadAOIs(const std::string &path, int maxRecords = -1);

private:
  std::vector<std::shared_ptr<const model::Scene>> scenes;
  std::vector<std::shared_ptr<const model::AOI>> aois;
};
}
}

#endif