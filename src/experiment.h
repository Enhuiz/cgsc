#ifndef CGSC_EXPERIMENT_H
#define CGSC_EXPERIMENT_H

#include <vector>

#include "solution.h"
#include "csv.hpp"
#include "json.hpp"

struct Loader
{
  std::vector<std::unique_ptr<AOI>> aois;
  std::vector<std::unique_ptr<Scene>> scenes;
  Loader(const std::string &aois_path, const std::string &scenes_path);
  std::vector<Scene *> get_scenes() const;
  std::vector<AOI *> get_aois() const;
};

struct Analysor
{
  bool cell_enabled;
  bool polygon_enabled;
  nlohmann::json get_aoi_report(const AOI *aoi) const;
  nlohmann::json get_scene_report(const Scene *scene) const;
  nlohmann::json get_scenes_report(const std::vector<Scene *> scenes) const;
  double calculate_coverage_ratio(const AOI *aoi, const std::vector<Scene *> &scenes) const;
};

nlohmann::json query(AOI *aoi, const std::vector<Scene *> &scenes, double delta);
nlohmann::json experiment(const std::string &aois_path, const std::string &scenes_path, double delta);

#endif