#ifndef CGSC_EXPERIMENT_H
#define CGSC_EXPERIMENT_H

#include "solution.h"
#include "csv.hpp"
#include "json.hpp"

struct Loader
{
  std::vector<std::unique_ptr<AOI>> aois;
  std::vector<std::unique_ptr<Scene>> scenes;
  Loader(const std::string &aois_path, const std::string &scenes_path);
  std::vector<Scene *> get_scenes();
  std::vector<Scene *> get_aois();
};

struct Analysor
{
  bool cell_enabled;
  bool polygon_enabled;
  nlohmann::json get_aoi_report(const AOI *aoi);
  nlohmann::json get_scene_report(const Scene *scene);
  nlohmann::json get_scenes_report(const std::vector<Scene *> scenes);
  double calculate_coverage_ratio(const AOI *aoi, const std::vector<*Scene> &scenes);
};

nlohmann::json query(AOI *aoi, const std::vector<*Scene> &scenes, double delta);
nlohmann::json experiment(const std::string &aois_path, const std::string &scenes_path, double delta);

#endif