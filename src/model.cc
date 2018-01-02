#include "model.h"

#include <iostream>
#include <fstream>
#include <set>

using namespace std;

bool operator==(const Element &a, const Element &b)
{
    return a.id == b.id;
}

bool operator<(const Element &a, const Element &b)
{
    return a.id < b.id;
}

void Set::update_value()
{
    value = func::sum(elements, [](const Element &element) {
        return element.value;
    });
}

Set::~Set() {}

void Range::update_cost()
{
    cost = product->price;
}
