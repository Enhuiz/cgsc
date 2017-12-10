#include "model.h"

#include <iostream>
#include <fstream>
#include <set>

using namespace std;

bool operator==(const Element &a, const Element &b)
{
    return a.index == b.index;
}

Range::Range(Range &&range)
    : entity(range.entity), elements(move(range.elements)), value(range.value), cost(range.cost)
{
}

void Range::update_cost()
{
    cost = entity->price;
}

void Range::update_value()
{
    value = func::sum(elements, [](const Element &element) {
        return element.value;
    });
}

