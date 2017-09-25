#pragma once
#ifndef CGSC_SOLVER_RESULT_H
#define CGSC_SOLVER_RESULT_H

#include <list>
#include <string>
#include <iostream>

#include "cgsc/model/scene.h"
#include "cgsc/model/aoi.h"
#include "cgsc/model/polygon.h"

namespace cgsc
{
namespace solver
{
class Result
{
public:
  Result(std::shared_ptr<model::AOI> aoi, const std::list<std::shared_ptr<model::Scene>> &scenes);

  double getCoverageArea() const;

  double getAOIgetArea() const;

  double getCoverageRatio() const;

  double getPrice() const;

  std::shared_ptr<const model::AOI> getAOI() const;

  void save(const std::string& path) const;

public:
  friend std::ostream &operator<<(std::ostream &os, const Result &result);

private:
  std::list<std::shared_ptr<model::Scene>> scenes;
  std::shared_ptr<model::AOI> aoi;

  double price;
  double coverageArea;
};
}
}

#endif