#ifndef CGSC_EXPERIMENT_H
#define CGSC_EXPERIMENT_H

#include <string>

#include "json.hpp"

nlohmann::json experiment(const std::string &rois_path, const std::string &scenes_path, double target_coverage_ratio, double delta);

#endif