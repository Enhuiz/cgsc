#ifndef CGSC_MODEL_H
#define CGSC_MODEL_H

#include "geometry.h"

using CID = unsigned long long;
using CellSet = std::set<CID>;

struct AOI
{
    std::string s;
    Polygon poly;
    CellSet cell_set;              // discrete
    std::list<Polygon> offcuts; // continuous
};

struct Scene
{
    std::string s;
    Polygon poly;
    double price;
    CellSet cell_set;
    std::list<Polygon> offcuts;
};

#endif