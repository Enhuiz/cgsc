#ifndef CGSC_MODEL_H
#define CGSC_MODEL_H

#include <functional>

#include "json.hpp"

#include "geometry.h"
#include "global.h"

using CID = unsigned long long;
using CellSet = std::set<CID>;

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


#endif