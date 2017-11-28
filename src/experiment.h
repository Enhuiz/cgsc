#ifndef CGSC_EXPERIMENT_H
#define CGSC_EXPERIMENT_H

#include <string>

#include "json.hpp"

nlohmann::json experiment(const std::string &rois_dir, const std::string &archive_dir, const nlohmann::json &setting);

#endif