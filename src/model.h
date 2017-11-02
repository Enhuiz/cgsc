#ifndef CGSC_MODEL_H
#define CGSC_MODEL_H

#include <functional>
#include <unordered_set>

#include "json.hpp"

#include "geometry.h"
#include "global.h"

using CID = unsigned long long;
using CellSet = std::unordered_set<CID>;

struct Model
{
    std::string s;
    Polygon poly;
    CellSet cell_set;
    std::list<Polygon> offcuts;
};

struct AOI : Model
{
};

struct Scene : Model
{
    double price;
};

namespace discrete
{
void discretize_aoi(AOI *aoi, double delta);
void discretize_scenes(const std::list<Scene *> &scenes, AOI *aoi, double delta);
}

namespace continuous
{
void cut_aoi(AOI *aoi);
void cut_scenes(const std::list<Scene *> &scenes, AOI *aoi);
double area(const std::list<Polygon> &offcuts);
double calculate_coverage_ratio(AOI *aoi, const std::list<Scene *> &scenes);
}

#endif