#ifndef CGSC_SOLUTION_H
#define CGSC_SOLUTION_H

#include <list>

#include "model.h"

std::list<Scene *> select_possible_scenes(ROI *roi, const std::list<Scene *> &scenes);

namespace discrete
{
namespace greedy
{
std::list<Scene *> optimize(ROI *roi, std::list<Scene *> possible_scenes, double delta);
}
}

namespace continuous
{
namespace greedy
{
std::list<Scene *> optimize(ROI *roi, std::list<Scene *> possible_scenes, double delta);
}
namespace branch_and_bound
{
std::list<Scene *> optimize(ROI *roi, std::list<Scene *> possible_scenes, double delta);
}
}

#endif