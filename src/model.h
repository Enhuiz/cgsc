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
    double unit_price;
};

void remove_scenes_with_no_cells(std::list<Scene *> &scenes);
void remove_scenes_with_no_offcuts(std::list<Scene *> &scenes);
void remove_tiny_offcuts(std::list<Polygon> &offcuts, double delta);
double area(const std::list<Polygon> &offcuts);
double area(const CellSet &cell_set, const std::vector<double> &area_table);
double calculate_coverage_ratio(ROI *roi, const std::list<Scene *> &scenes);
nlohmann::json to_json(const std::list<Polygon> &polys);

namespace discrete
{
void transform(ROI *roi, std::list<Scene *> &scenes, double delta);
}

namespace continuous
{
void transform(ROI *roi, std::list<Scene *> &scenes, double delta);
}

namespace semantical
{
std::vector<double> transform(ROI *roi, std::list<Scene *> &scenes, double delta);
};

#endif