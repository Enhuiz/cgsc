#ifndef CGSC_EXPERIMENT_H
#define CGSC_EXPERIMENT_H

#include <string>

#include "json.hpp"

void experiment(const std::string &rois_dir,
                const std::string &products_dir,
                const std::string &output_path,
                const nlohmann::json &settings);

#endif