#include "solver.h"

#include "global.h"

using namespace std;
using nlohmann::json;

Solver::Solver(shared_ptr<Transformer> transformer, shared_ptr<Optimizer> optimizer)
    : transformer(transformer), optimizer(optimizer)
{
}

Products Solver::preprocess(const Roi &roi, const Products &products) const
{
    // select possible products and crop them
    auto possible_products = Products();
    for (const auto &product : products)
    {
        auto inner_polygons = intersection(roi.polygon, product.polygon);
        if (inner_polygons.size() > 0)
        {
            auto possible_product = product;
            possible_product.polygon = inner_polygons.front();
            possible_products.push_back(possible_product);
        }
    }
    return possible_products;
}

json Solver::solve(const Roi &roi, const Products &products, Products &&result_products) const
{
    auto report = json();
    if (transformer == nullptr)
    {
        // cerr << "transformer not found" << endl;
        return report;
    }
    report["transformer"] = transformer->tag();
    report["area_of_roi"] = area(roi.polygon);
    auto possible_products = preprocess(roi, products);
    report["number_of_possible_products"] = possible_products.size();
    auto sw = Stopwatch();
    auto universe = Universe();
    auto ranges = Ranges();
    auto result_ranges = Ranges();
    report["transformation"] = transformer->transform(roi, possible_products, universe, ranges);
    report["transformation"]["time"] = sw.lap();
    report["number_of_cells"] = universe.elements.size();

    if (optimizer == nullptr)
    {
        // cerr << "optimizer not found" << endl;
        return report;
    }

    report["optimizer"] = optimizer->tag();
    sw.restart();
    report["optimization"] = optimizer->optimize(universe, ranges, result_ranges);
    report["optimization"]["time"] = sw.lap();
    if (result_ranges.size() > 0)
    {
        report["optimization"]["cost"] = func::sum(result_ranges, [](const auto &result_range) {
            return result_range.cost;
        });
    }
    else
    {
        report["optimization"]["cost"] = numeric_limits<double>::quiet_NaN();
    }

    result_products = func::map(result_ranges, [](const auto &result_range) {
        return *result_range.entity;
    });

    return report;
}

string Solver::tag() const
{
    auto transformer_tag = transformer == nullptr ? "None" : transformer->tag();
    auto optimizer_tag = optimizer == nullptr ? "None" : optimizer->tag();
    return transformer_tag + "::" + optimizer_tag;
}