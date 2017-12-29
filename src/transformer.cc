#include "transformer.h"

#include <unordered_map>
#include <set>

using namespace std;
using nlohmann::json;

json Transformer::transform(const Roi &roi,
                            const Products &products,
                            Universe &universe,
                            Ranges &ranges) const
{
    report.clear();

    // initialize
    universe.roi = &roi;
    ranges = func::map(products, [](const Product &product) {
        auto range = Range();
        range.product = &product;
        range.update_cost();
        return range;
    });

    transform_impl(roi, products, universe, ranges);

    universe.update_value();
    for (auto &range : ranges)
        range.update_value();

    return report;
}

void Transformer::add_imagery_cell(Universe &universe) const
{
    // add imagery cell
    double uncovered_area = area(universe.roi->polygon) - func::sum(universe.elements,
                                                                    [](const Element &e) {
                                                                        return e.value;
                                                                    });

    report["uncovered_percentage"] = uncovered_area / area(universe.roi->polygon);
    universe.elements.insert(Element{static_cast<int>(universe.elements.size()), uncovered_area});
}

void OnlineTranformer::transform_impl(const Roi &roi,
                                      const Products &products,
                                      Universe &universe,
                                      Ranges &ranges) const

{
}

DiscreteTransformer::DiscreteTransformer(double delta) : delta(delta)
{
}

unordered_set<int> DiscreteTransformer::discretize(const Polygon &polygon,
                                                   function<bool(const Polygon &)> condition) const
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
                ret.insert(unique_index(grid_cell));
            }
        }
    }
    return ret;
}

int DiscreteTransformer::unique_index(const Polygon &grid_cell) const
{
    auto lower_left = grid_cell.front();
    uint i = lower_left.x / delta;
    uint j = lower_left.y / delta;
    assert(j < (1 << 16) && i < (1 << 16) && "index overflow");
    return i + (j << 16);
}

void DiscreteTransformer::transform_impl(const Roi &roi,
                                         const Products &products,
                                         Universe &universe,
                                         Ranges &ranges) const
{
    auto element_value = delta * delta;
    auto id_map = unordered_map<int, int>();
    for (auto &range : ranges)
    {
        auto indexes = discretize(range.product->polygon, [&range](const Polygon &grid_cell) {
            return all_of(grid_cell.begin(), grid_cell.end(), [&range](const Point &p) { return !outside(p, range.product->polygon); });
        });
        for (auto index : indexes)
        {
            if (id_map.count(index) == 0)
            {
                int new_id = id_map.size();
                id_map[index] = new_id;
            }
            auto id = id_map[index];
            range.elements.insert(Element{id, element_value});
            universe.elements.insert(Element{id, element_value});
        }
    }
    add_imagery_cell(universe);
}

void ContinuousTransformer::transform_impl(const Roi &roi,
                                           const Products &products,
                                           Universe &universe,
                                           Ranges &ranges) const
{

    struct Cell
    {
        Polygon polygon;     // geometric shape of the cell
        set<Range *> owners; // the rectangles that cover the cell
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

        auto new_inner_cells = list<Cell>();
        auto new_outer_cells = list<Cell>();
        for (auto it = cells.begin(); it != cells.end();)
        {
            auto inners = list<Polygon>();
            auto outers = list<Polygon>();
            tie(inners, outers) = clip(it->polygon, range.product->polygon);
            if (inners.size() > 0) // the product intersects with the cell
            {
                auto prev_owners = move(it->owners); // record the owner of the cell
                it = cells.erase(it);                // discard the cell
                for (auto outer : outers)
                {
                    new_outer_cells.push_back(Cell{outer, prev_owners});
                }
                prev_owners.insert(&range); // the inners are also covered by the product itself
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

        auto range_polygons = difference({range.product->polygon},
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

        // clear_online_plotter();
        // for (const auto &cell : cells)
        // {
        //     append_polygon_to_online_plotter(to_string(cell.polygon));
        // }
        // sleep(250);
    }

    // calculate the value (i.e. area) of each cell
    auto value_map = map<set<Range *>, double>();
    for (const auto &cell : cells)
    {
        if (value_map.count(cell.owners) == 0)
        {
            value_map[cell.owners] = 0;
        }
        value_map[cell.owners] += area(cell.polygon);
    }

    for (const auto &kv : value_map)
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

    add_imagery_cell(universe);
    // rectangles per cell
    // report["histogram"] = {};
    // for (const auto& kv: value_map)
    // {
    //     report["histogram"].push_back(kv.first.size());
    // }

    // cells per rectangle
    // report["histogram"] = func::map(value_map, [](const Range &range) {
    //     return range.elements.size();
    // });
}

void FastContinuousTransformer::transform_impl(const Roi &roi,
                                               const Products &products,
                                               Universe &universe,
                                               Ranges &ranges) const
{
    struct Cell;

    struct Node
    {
        Range &range;
        const Polygon &polygon; // make reference easier, it is actually range.product->polygon
        list<Cell> cells;
        set<int> neighboors;
    };

    struct Cell
    {
        Polygon polygon;     // geometric shape of the cell
        set<Range *> owners; // owned by nodes
        void clipped_by(const Node &node, list<Cell> &inners, list<Cell> &outers)
        {
            auto inner_polygons = list<Polygon>();
            auto outer_polygons = list<Polygon>();
            tie(inner_polygons, outer_polygons) = clip(polygon, node.polygon);
            auto union_owners = owners;
            union_owners.insert(&(node.range));
            for (const auto &inner_polygon : inner_polygons)
            {
                inners.push_back(Cell{
                    inner_polygon,
                    union_owners,
                });
            }
            for (const auto &outer_polygon : outer_polygons)
            {
                outers.push_back(Cell{
                    outer_polygon,
                    owners,
                });
            }
        }
    };

    auto nodes = vector<Node>();
    nodes.reserve(ranges.size());

    for (auto &range : ranges)
    {
        auto node = Node{
            range,
            range.product->polygon,
        };
        nodes.push_back(node);
    }

    int cnt_its = 0;
    for (int i = 0; i < nodes.size(); ++i)
    {
        auto &ni = nodes[i];
        for (int j = i + 1; j < nodes.size(); ++j)
        {
            auto &nj = nodes[j];
            if (intersects(ni.polygon, nj.polygon))
            {
                ni.neighboors.insert(j);
                nj.neighboors.insert(i);
                cnt_its++;
            }
        }
    }

    vector<bool> visited(nodes.size(), false);

    // auto num_unvisited_neighboor = [&visited](const Node& node) {
    //     return area(node.polygon);
    //     // int cnt = 0;
    //     // for (auto neighboor : node.neighboors)
    //     // {
    //     //     if (visited[neighboor] == false)
    //     //     {
    //     //         ++cnt;
    //     //     }
    //     // }
    //     // return cnt;
    // };

    for (int i = 0; i < nodes.size(); ++i)
    {
        auto &node = nodes[i];
        auto cutters = list<Polygon>();
        for (auto nid : node.neighboors)
        {
            if (visited[nid])
            {
                auto &neighboor_node = nodes[nid];
                auto new_cells_for_neighboor = list<Cell>();
                for (auto cit = neighboor_node.cells.begin();
                     cit != neighboor_node.cells.end();)
                {
                    auto inners = list<Cell>();
                    auto outers = list<Cell>();
                    cit->clipped_by(node, inners, outers);
                    if (inners.size() > 0) // there are intersection!
                    {
                        if (true)
                        { // give it to the new one
                            node.cells.insert(node.cells.end(),
                                              inners.begin(),
                                              inners.end());
                        }
                        else
                        {
                            new_cells_for_neighboor.insert(new_cells_for_neighboor.end(),
                                                           inners.begin(),
                                                           inners.end());
                        }
                        new_cells_for_neighboor.insert(new_cells_for_neighboor.end(),
                                                       outers.begin(),
                                                       outers.end());
                        for (const auto &inner : inners)
                        {
                            cutters.push_back(inner.polygon);
                        }
                        cit = neighboor_node.cells.erase(cit); // erase the old one
                    }
                    else
                    {
                        ++cit;
                    }
                }
                neighboor_node.cells.insert(neighboor_node.cells.end(),
                                            new_cells_for_neighboor.begin(),
                                            new_cells_for_neighboor.end());
            }
        }
        auto cut_parts = difference({node.polygon}, cutters);
        for (const auto &cut_part : cut_parts)
        {
            node.cells.push_back(Cell{
                cut_part,
                {&node.range},
            });
        }
        visited[i] = true;

        // clear_online_plotter();
        // for (const auto& node: nodes)
        // {
        //     for (const auto& cell: node.cells)
        //     {
        //         append_polygon_to_online_plotter(to_string(cell.polygon));
        //     }
        // }
        // sleep(250);
    }

    auto cells = list<Cell>();
    for (const auto &node : nodes)
    {
        cells.insert(cells.end(), node.cells.begin(), node.cells.end());
    }

    // calculate the value (i.e. area) of each cell
    auto value_map = map<set<Range *>, double>();
    for (const auto &cell : cells)
    {
        if (value_map.count(cell.owners) == 0)
        {
            value_map[cell.owners] = 0;
        }
        value_map[cell.owners] += area(cell.polygon);
    }

    for (const auto &kv : value_map)
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

    add_imagery_cell(universe);
}

// auto range_id = map<Range *, int>();

// struct Cell
// {
//     Polygon polygon;     // geometric shape of the cell
//     set<Range *> owners; // the rectangles that cover the cell
//     double value;

//     string owners_str() const
//     {
//         string ret;
//         for (auto owner : owners)
//         {
//             ret += to_string(range_id[owner]) + ", ";
//         }
//         ret.pop_back();
//         ret.pop_back();
//         return ret;
//     }
// };

// bool operator<(const Cell &a, const Cell &b)
// {
//     return a.owners < b.owners;
// }

// list<Cell> intersection(const Cell &c1, const Cell &c2)
// {
//     auto ret = list<Cell>();
//     auto inners = intersection(c1.polygon, c2.polygon);
//     auto owners = c1.owners;
//     owners.insert(c2.owners.begin(), c2.owners.end());
//     for (const auto &inner : inners)
//     {
//         ret.push_back(Cell{
//             inner,
//             owners,
//         });
//     }
//     return ret;
// }

// set<Cell> divide(const Cell &region, const list<Cell> &cells)
// {
//     static map<set<Range *>, set<Cell>> memorized;
//     static set<Cell> total_divided_cells;

//     if (memorized.count(region.owners) > 0)
//     {
//         return set<Cell>();
//     }

//     auto deduced_cells = cells;
//     deduced_cells.unique([](const Cell& a, const Cell& b){
//         return a.polygon == b.polygon;
//     });
//     cout << deduced_cells.size()  << " == " << cells.size() << endl;

//     auto divided_cells = set<Cell>();
//     for (const auto &sub_region : cells)
//     {
//         auto inners = list<Cell>();
//         for (const auto &cell : cells)
//         {
//             if (&sub_region != &cell)
//             {
//                 auto new_inners = intersection(sub_region, cell);
//                 inners.insert(inners.end(), new_inners.begin(), new_inners.end());
//             }
//         }
//         auto divided_cells_inside_sub_region = divide(sub_region, inners);
//         divided_cells.insert(divided_cells_inside_sub_region.begin(),
//                              divided_cells_inside_sub_region.end());
//     }

//     double divided_cells_area = func::sum(divided_cells, [](const Cell &cell) {
//         return cell.value;
//     });

//     auto imagery_cell = Cell();
//     imagery_cell.owners = region.owners;
//     imagery_cell.value = area(region.polygon) - divided_cells_area;
//     if (imagery_cell.value > 0)
//     {
//         divided_cells.insert(imagery_cell);
//         total_divided_cells.insert(divided_cells.begin(), divided_cells.end());
//         cout << "region owners: " << region.owners_str() << endl;
//         cout << "region area: " << area(region.polygon) << endl;
//         cout << "total cells: " << total_divided_cells.size() << endl;
//         cout << "total value: " << func::sum(total_divided_cells, [](const Cell &cell) {
//             return cell.value;
//         }) << endl;
//         assert(divided_cells_area < area(region.polygon) && "divided cells too large");
//     }
//     return memorized[region.owners] = divided_cells;
// }

// void FailedContinuousTransformer::transform_impl(const Roi &roi,
//                                                const Products &products,
//                                                Universe &universe,
//                                                Ranges &ranges) const
// {
//     auto region = Cell();
//     region.polygon = roi.polygon;
//     region.owners = {&universe};
//     auto cells = list<Cell>();
//     for (auto &range : ranges)
//     {
//         if (range_id.count(&range) == 0)
//         {
//             range_id[&range] = range_id.size();
//         }
//         auto inner_polygons = intersection(range.product->polygon, roi.polygon);
//         for (const auto &inner_polygon : inner_polygons)
//         {
//             auto cell = Cell();
//             cell.polygon = inner_polygon;
//             cell.owners = {&universe, &range};
//             cells.push_back(cell);
//         }
//     }

//     divide(region, cells);
// }
