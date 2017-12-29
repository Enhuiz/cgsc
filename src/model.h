#ifndef CGSC_MODEL_H
#define CGSC_MODEL_H

#include <functional>
#include <unordered_set>
#include <set>

#include "json.hpp"

#include "geometry.h"
#include "global.h"

struct Entity
{
    Polygon polygon;
    double price;
    Entity(Polygon polygon, double price) : polygon(std::move(polygon)), price(price) {}
};

using Roi = Entity;
using Product = Entity;
using Rois = std::vector<Roi>;
using Products = std::vector<Product>;

struct Element // element of set cover problem
{
    int id;
    double value;
};

bool operator==(const Element &a, const Element &b);
bool operator<(const Element &a, const Element &b);

namespace std
{
template <>
struct hash<Element>
{
    using argument_type = Element;
    using result_type = std::size_t;
    result_type operator()(argument_type const &element) const noexcept
    {
        return element.id;
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