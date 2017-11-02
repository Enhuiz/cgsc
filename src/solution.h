#ifndef CGSC_SOLUTION_H
#define CGSC_SOLUTION_H

#include <list>

#include "model.h"

std::list<Scene *> select_possible_scenes(AOI *aoi, const std::list<Scene *> &scenes);

namespace discrete
{
std::list<Scene *> select_approx_optimal_scenes(AOI *aoi, std::list<Scene *> possible_scenes, double delta);
}

namespace continuous
{
std::list<Scene *> select_approx_optimal_scenes(AOI *aoi, std::list<Scene *> possible_scenes, double delta);
}

namespace brute_force
{
std::list<Scene *> select_optimal_scenes(AOI *aoi, std::list<Scene *> possible_scenes, double delta);
}

#endif