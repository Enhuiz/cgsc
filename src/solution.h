#ifndef CGSC_SOLUTION_H
#define CGSC_SOLUTION_H

#include <vector>

#include "geometry.h"

std::vector<Scene *> select_possible_scenes(AOI *aoi, const std::vector<Scene *> &scenes);
void discretize_aoi(const Discretizer &discretizer, AOI *aoi);
void discretize_scenes(const Discretizer &discretizer, AOI *aoi, const std::vector<Scene *> &scenes);
std::vector<Scene *> select_approx_optimal_scenes(const AOI *aoi, const std::vector<Scene *> &scenes);

#endif