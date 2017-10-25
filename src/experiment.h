#ifndef CGSC_EXPERIMENT_H
#define CGSC_EXPERIMENT_H

#include <string>

#include "json.hpp"

nlohmann::json experiment(const std::string &aois_path, const std::string &scenes_path, double delta);

#endif