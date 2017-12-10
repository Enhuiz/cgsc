#ifndef CGSC_MODEL_H
#define CGSC_MODEL_H

#include <functional>
#include <unordered_set>

#include "json.hpp"

#include "geometry.h"
#include "global.h"

struct Entity // meta data for Roi and image products
{
    Polygon polygon; // polygon, a vector of points
    double price;
};

using Roi = Entity;
using Rois = std::vector<Roi>;
using Product = Entity;
using Products = std::vector<Product>;

struct Element // element of set cover problem
{
    int index;
    double value;
};

bool operator==(const Element &a, const Element &b);

namespace std
{
template <>
struct hash<Element>
{
    using argument_type = Element;
    using result_type = std::size_t;
    result_type operator()(argument_type const &element) const noexcept
    {
        return element.index;
    }
};
}

struct Range // range (i.e. subset) of set cover problem
{
    const Entity *entity;
    std::unordered_set<Element> elements;
    double value;
    double cost;
    Range() = default;
    Range(const Range &range) = default;
    Range(Range &&range);
    Range &operator=(const Range &range) = default;
    void update_value();
    void update_cost();
};

using Universe = Range;
using Ranges = std::vector<Range>;

#endif