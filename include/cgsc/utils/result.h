#pragma once
#ifndef CGSC_SOLVER_RESULT_H
#define CGSC_SOLVER_RESULT_H

#include <list>
#include <string>
#include <iostream>
#include <map>

#include "json.hpp"

#include "cgsc/model/scene.h"
#include "cgsc/model/aoi.h"
#include "cgsc/model/polygon.h"

namespace cgsc
{
namespace utils
{
class Result
{
public:
  void addPossibleScenes(const std::vector<std::shared_ptr<const model::Scene>> &scenes, bool verbose);
  void addTotalPrice(const std::vector<std::shared_ptr<const model::Scene>> &scenes);
  void addCoverageArea(const model::AOI &aoi, const std::vector<std::shared_ptr<const model::Scene>> &scenes);
  void addResultScense(const std::vector<std::shared_ptr<const model::Scene>> &scenes, bool verbose);
  void addAOI(const model::AOI &aoi, bool verbose);
  void addJSON(const std::string &tag, const nlohmann::json &j);

  nlohmann::json toJSON() const;

public:
  friend std::ostream &operator<<(std::ostream &os, const Result &result);

private:
  nlohmann::json jobj;
};
}
}

#endif