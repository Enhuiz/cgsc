#ifndef CGSC_SOLUTION_H
#define CGSC_SOLUTION_H

#include <list>

#include "model.h"

std::list<Scene *> select_possible_scenes(AOI *aoi, const std::list<Scene *> &scenes);

namespace discrete
{
void discretize_aoi(AOI *aoi, double delta);
void discretize_scenes(const std::list<Scene *> &scenes, AOI *aoi, double delta);
std::list<Scene *> select_approx_optimal_scenes(AOI *aoi, const std::list<Scene *> &scenes);
}

namespace continuous
{
void triangulate_aoi(AOI *aoi);
void triangulate_scenes(const std::list<Scene *> &scenes, AOI *aoi);
std::list<Scene *> select_approx_optimal_scenes(AOI *aoi, const std::list<Scene *> &scenes);
}

#endif