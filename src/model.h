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
    Entity *entity;
    std::unordered_set<Element> elements;

    double value() const;
    double cost() const;
};

struct Transformer
{
    nlohmann::json report;
    std::unique_ptr<Range> universe;
    std::list<std::unique_ptr<Range>> ranges;
};

struct Discrete : Transformer
{
    Discrete(Entity *roi,
             std::list<Entity *> records,
             double delta);

    static std::string tag() { return "discrete"; }
};

struct Continuous : Transformer
{
    Continuous(Entity *roi,
               std::list<Entity *> records,
               double delta);

    static std::string tag() { return "continuous"; }
};

#endif