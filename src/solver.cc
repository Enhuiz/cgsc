#include "solver.h"

#include "global.h"

using namespace std;
using nlohmann::json;

Solver::Solver(shared_ptr<Transformer> transformer, shared_ptr<Optimizer> optimizer)
    : transformer(transformer), optimizer(optimizer)
{
}

json Solver::solve(const Roi &roi, const Products &products, Products &&result_products) const
{
    auto report = json();
    if (transformer == nullptr)
    {
        // cerr << "transformer not found" << endl;
        return report;
    }

    report["solver"] = tag();

    report["area of roi"] = area(roi.polygon);
    auto possible_products = func::filter(products, [&roi](const Product &product) {
        return intersects(product.polygon, roi.polygon);
    });
    report["number of possible products"] = possible_products.size();

    auto sw = Stopwatch();
    auto universe = Universe();
    auto ranges = Ranges();
    auto result_ranges = Ranges();
    report["transformer"] = transformer->transform(roi, possible_products, universe, ranges);
    report["transformer"]["time"] = sw.lap();

    if (optimizer == nullptr)
    {
        // cerr << "optimizer not found" << endl;
        return report;
    }

    sw.restart();
    report["optimizer"] = optimizer->optimize(universe, ranges, result_ranges);
    report["optimizer"]["time"] = sw.lap();
    report["optimizer"]["cost"] = func::sum(result_ranges, [](const auto &result_range) {
        return result_range.cost;
    });

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