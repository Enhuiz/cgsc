#ifndef CGSC_MODEL_H
#define CGSC_MODEL_H

#include <functional>
#include <unordered_set>
#include <set>

#include "json.hpp"

#include "geometry.h"
#include "global.h"

struct Region
{
    Region(Polygon polygon) : polygon(std::move(polygon)) {}
    Polygon polygon;
};

struct Roi : public Region
{
    Roi(Polygon polygon) : Region(std::move(polygon)) {}
};

struct Product : public Region
{
    Product(Polygon polygon, double price) : Region(std::move(polygon)), price(price) {}
    double price;
};

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

struct Set
{
    std::unordered_set<Element> elements;
    double value;
    virtual void update_value();
};

struct Universe : Set
{
    const Roi *roi;
};

struct Range : Set
{
    const Product *product;
    double cost;
    void update_cost();
};

using Ranges = std::vector<Range>;

#endif