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

struct ROI : Model
{
};

struct Scene : Model
{
    double price;
    double valid_area; // price / area of region inside the ROI
};

namespace discrete
{
void discretize_roi(ROI *roi, double delta);
void discretize_scenes(const std::list<Scene *> &scenes, ROI *roi, double delta);
void remove_scenes_with_empty_cell_set(std::list<Scene *> &scenes);
}

namespace continuous
{
void cut_roi(ROI *roi);
void cut_scenes(const std::list<Scene *> &scenes, ROI *roi);
double area(const std::list<Polygon> &offcuts);
double calculate_coverage_ratio(ROI *roi, const std::list<Scene *> &scenes);
void remove_tiny_offcuts(std::list<Polygon> &offcuts, double delta);
void remove_scenes_with_no_offcuts(std::list<Scene *> &scenes);
nlohmann::json to_json(const std::list<Polygon> &polys);
}

#endif