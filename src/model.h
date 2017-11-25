#ifndef CGSC_MODEL_H
#define CGSC_MODEL_H

#include <functional>
#include <unordered_set>

#include "json.hpp"

#include "geometry.h"
#include "global.h"

struct Entity
{
    std::string s;
    Polygon poly;
    double price;
};

struct Element
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
    result_type operator()(argument_type const &e) const noexcept
    {
        return std::hash<int>{}(e.index);
    }
};
}

struct Range
{
    const Entity *entity;
    std::unordered_set<Element> elements;
    double value;
    double cost;
    void update_value();
    void update_cost();
};

struct Transformer
{
    nlohmann::json report;
    Range universe;
    std::list<Range> ranges;

    Transformer(const Entity &roi,
                const std::list<Entity> &records,
                double delta);
};

struct Geometric : Transformer
{
    Geometric(const Entity &roi,
             const std::list<Entity> &records,
             double delta);

    static std::string tag() { return "geometric"; }
};

struct Discrete : Transformer
{
    Discrete(const Entity &roi,
             const std::list<Entity> &records,
             double delta);

    static std::string tag() { return "discrete"; }
};

struct Continuous : Transformer
{
    Continuous(const Entity &roi,
               const std::list<Entity> &records,
               double delta);

    static std::string tag() { return "continuous"; }
};

#endif