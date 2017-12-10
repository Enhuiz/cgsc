#include "transformer.h"

using namespace std;
using nlohmann::json;

json Transformer::transform(const Roi &roi,
                            const Products &products,
                            Range &universe,
                            Ranges &ranges)
{
    auto report = json();
    // initialize
    universe.entity = &roi;
    ranges = func::map(products, [](const Product &product) {
        auto range = Range();
        range.entity = &product;
        range.update_cost();
        return range;
    });

    transform_impl(roi, products, universe, ranges, report);

    // add imagery cell
    double uncovered_area = area(roi.polygon) - func::sum(universe.elements,
                                                          [](const Element &e) {
                                                              return e.value;
                                                          });

    universe.elements.insert(Element{static_cast<int>(universe.elements.size()), uncovered_area});
    report["uncovered"] = uncovered_area / area(roi.polygon);
    report["number of cells"] = universe.elements.size();

    universe.update_value();
    for (auto &range : ranges)
        range.update_value();
    return report;
}

DiscreteTransformer::DiscreteTransformer(double delta) : delta(delta)
{
}

unordered_set<int> DiscreteTransformer::discretize(const Polygon &polygon, function<bool(const Polygon &)> condition)
{
    double minx, miny, maxx, maxy;
    minx = maxx = polygon.begin()->x;
    miny = maxy = polygon.begin()->y;
    for (const auto &p : polygon)
    {
        minx = std::min(minx, p.x);
        miny = std::min(miny, p.y);
        maxx = std::max(maxx, p.x);
        maxy = std::max(maxy, p.y);
    }
    minx = floor(minx / delta) * delta;
    miny = floor(miny / delta) * delta;
    maxx = ceil(maxx / delta) * delta;
    maxy = ceil(maxy / delta) * delta;
    auto ret = unordered_set<int>();
    for (double x = minx; x < maxx; x += delta)
    {
        for (double y = miny; y < maxy; y += delta)
        {
            auto grid_cell = box({x, y}, {x + delta, y + delta});
            if (condition(grid_cell))
            {
                ret.insert(hash(grid_cell));
            }
        }
    }
    return ret;
}

int DiscreteTransformer::hash(const Polygon &grid_cell)
{
    auto lower_left = grid_cell.front();
    uint i = lower_left.x / delta;
    uint j = lower_left.y / delta;
    assert(j < (1 << 16) && i < (1 << 16) && "index overflow");
    return i + (j << 16);
}

void DiscreteTransformer::transform_impl(const Roi &roi,
                                         const Products &products,
                                         Range &universe,
                                         Ranges &ranges,
                                         json &report)
{
    auto element_value = delta * delta;
    for (auto &range : ranges)
    {
        auto indexes = discretize(range.entity->polygon, [&universe, &range](const Polygon &grid_cell) {
            return any_of(grid_cell.begin(), grid_cell.end(), [&universe](const Point &p) { return !outside(p, universe.entity->polygon); }) &&
                   all_of(grid_cell.begin(), grid_cell.end(), [&range](const Point &p) { return !outside(p, range.entity->polygon); });
        });
        for (auto index : indexes)
        {
            range.elements.insert(Element{index, element_value});
            universe.elements.insert(Element{index, element_value});
        }
    }
}

void ContinuousTransformer::transform_impl(const Roi &roi,
                                           const Products &products,
                                           Range &universe,
                                           Ranges &ranges,
                                           json &report)
{

    struct Cell
    {
        Polygon polygon;        // geometric shape of the cell
        vector<Range *> owners; // the rectangles that cover the cell
    };

    auto cells = list<Cell>(); // cells currently got

    for (auto &range : ranges)
    {
        // for example, currently we have cell 1 and 2

        // ------------
        // | 1     /  |
        // |      /   |
        // |     / 2  |
        // ------------

        // and a new product

        //      -------------
        //     /           /
        //    ------------

        // the following process does this thing

        // ------------
        // | o     / o|
        // |      ----|--------
        // |     /  i |      /
        // -----------------

        // i for inner, o for outer. The 2 outer and 1 inner have been updated, and get

        // ------------
        // | 1'    /2'|
        // |      ----|--------
        // |     / 3' |      /
        // -----------------

        auto range_polygons = intersection(range.entity->polygon, roi.polygon); // fetch the products region that is inside Roi
        auto new_inner_cells = list<Cell>();
        auto new_outer_cells = list<Cell>();
        for (const auto &range_polygon : range_polygons) // for each inside region, in most case there is only 1 region
        {
            for (auto it = cells.begin(); it != cells.end();)
            {
                auto inners = list<Polygon>();
                auto outers = list<Polygon>();
                tie(inners, outers) = clip(it->polygon, range_polygon);
                if (inners.size() > 0) // the product intersects with the cell
                {
                    auto prev_owners = move(it->owners); // record the owner of the cell
                    it = cells.erase(it);                // discard the cell
                    for (auto outer : outers)
                    {
                        new_outer_cells.push_back(Cell{outer, prev_owners});
                    }
                    prev_owners.push_back(&range); // the inners are also covered by the product itself
                    for (auto inner : inners)
                    {
                        new_inner_cells.push_back(Cell{inner, prev_owners});
                    }
                }
                else
                {
                    ++it;
                }
            }
        }

        // the following difference does this things:

        // from
        //      -------------
        //     /  3'|      /
        //    ------------

        // to
        //      -------------
        //     /  3'|  4'  /
        //    ------------

        // so we get
        // ------------
        // | 1'    /2'|
        // |      ----|--------
        // |     / 3' |  4'  /
        // -----------------

        range_polygons = difference(range_polygons,
                                    func::map(new_inner_cells, // get all polygon of new inner cells
                                              [](const Cell &cell) {
                                                  return cell.polygon;
                                              }));

        for (const auto &range_polygon : range_polygons)
        {
            new_outer_cells.push_back(Cell{range_polygon, {&range}});
        }

        cells.splice(cells.end(), new_inner_cells);
        cells.splice(cells.end(), new_outer_cells);
    }

    // calculate the value (i.e. area) of each cell
    auto area_map = map<vector<Range *>, double>();
    for (const auto &cell : cells)
    {
        if (area_map.count(cell.owners) == 0)
        {
            area_map[cell.owners] = 0;
        }
        area_map[cell.owners] += area(cell.polygon);
    }

    for (const auto &kv : area_map)
    {
        int cell_index = universe.elements.size();
        // update universe
        universe.elements.insert(Element{cell_index, kv.second});

        // update ranges
        for (auto range : kv.first)
        {
            range->elements.insert(Element{cell_index, kv.second});
        }
    }

    // rectangles per cell
    // report["histogram"] = {};
    // for (const auto& kv: area_map)
    // {
    //     report["histogram"].push_back(kv.first.size());
    // }

    // cells per rectangle
    // report["histogram"] = func::map(area_map, [](const Range &range) {
    //     return range.elements.size();
    // });
}

void OnlineTranformer::transform_impl(const Roi &roi,
                                      const Products &products,
                                      Range &universe,
                                      Ranges &ranges,
                                      json &report)

{
}