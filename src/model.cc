#include "model.h"

#include <iostream>
#include <fstream>

using namespace std;

bool operator==(const Element &a, const Element &b)
{
    return a.index == b.index;
}

double Range::cost() const
{
    return entity->price;
}

double Range::value() const
{
    return func::sum(elements, [](const Element &e) {
        return e.value;
    });
}

static const auto floor_double = static_cast<double (*)(double)>(floor);
static const auto ceil_double = static_cast<double (*)(double)>(ceil);

struct Discretizer
{
    double delta;

    tuple<double, double, double, double> bounding_box_corner(const Polygon &poly,
                                                              const function<double(double)> &min_trunc,
                                                              const function<double(double)> &max_trunc) const
    {
        double minx, miny, maxx, maxy;
        minx = maxx = poly.begin()->x;
        miny = maxy = poly.begin()->y;
        for (const auto &p : poly)
        {
            minx = min(minx, p.x);
            miny = min(miny, p.y);
            maxx = max(maxx, p.x);
            maxy = max(maxy, p.y);
        }
        minx = min_trunc(minx / delta) * delta;
        miny = min_trunc(miny / delta) * delta;
        maxx = max_trunc(maxx / delta) * delta;
        maxy = max_trunc(maxy / delta) * delta;
        return make_tuple(minx, miny, maxx, maxy);
    }

    Polygon bounding_box(const Polygon &poly,
                         const function<double(double)> &min_trunc,
                         const function<double(double)> &max_trunc) const
    {
        double minx, miny, maxx, maxy;
        tie(minx, miny, maxx, maxy) = bounding_box_corner(poly, min_trunc, max_trunc);
        return {Point{minx, miny}, Point{maxx, miny}, Point{maxx, maxy}, Point{minx, maxy}};
    }

    Polygon get_cell_polygon(const Point &ll) const // lower left
    {
        double x = ll.x;
        double y = ll.y;
        return Polygon{{x, y},
                       {x + delta, y},
                       {x + delta, y + delta},
                       {x, y + delta}};
    }

    Polygon get_cell_polygon(const int &cid) const
    {
        return get_cell_polygon(cid_to_point(cid));
    }

    unordered_set<int> discretize(const Polygon &poly, bool keep_edge_cells) const
    {
        using ConditionType = function<bool(const Point &)>;

        ConditionType discard_edge = [&poly, this](const Point &ll) {
            auto cell_poly = get_cell_polygon(ll);
            return all_of(cell_poly.begin(), cell_poly.end(), [&poly](const Point &p) {
                return !outside(p, poly); // it's OK for cell points on edges, while out is not acceptable.
            });
        };

        ConditionType keep_edge = [&poly, this](const Point &ll) {
            return intersects(get_cell_polygon(ll), poly);
        };

        ConditionType condition = keep_edge_cells ? keep_edge : discard_edge;

        double minx, miny, maxx, maxy;
        tie(minx, miny, maxx, maxy) = keep_edge_cells ? bounding_box_corner(poly, floor_double, ceil_double) : bounding_box_corner(poly, ceil_double, floor_double);

        int minxi = round(minx / delta);
        int minyi = round(miny / delta);
        int maxxi = round(maxx / delta);
        int maxyi = round(maxy / delta);

        auto ret = unordered_set<int>();
        for (int i = minxi; i < maxxi; ++i)
        {
            for (int j = minyi; j < maxyi; ++j)
            {
                if (condition(Point{i * delta, j * delta}))
                {
                    ret.insert(index_to_cid(i, j));
                }
            }
        }
        return ret;
    }

    int index_to_cid(int xi, int yi) const
    {
        return xi + (yi << 16);
    }

    Point cid_to_point(const int &cid) const
    {
        int xi = (uint16_t)cid & (~0);
        int yi = cid >> 16;
        return {xi * delta, yi * delta};
    }
};

Discrete::Discrete(Entity *roi, std::list<Entity *> records, double delta)
{
    Discretizer discretizer{delta};

    { // initialize universe
        universe = unique_ptr<Range>(new Range{});
        auto eids = discretizer.discretize(roi->poly, true);
        universe->entity = roi;
        universe->elements.clear();
        for (auto eid : eids)
        {
            universe->elements.insert(Element{eid, 1});
        }
    }

    { // initialize sets
        auto roi_bounding_box = discretizer.bounding_box(roi->poly, floor_double, ceil_double);
        for (auto record : records)
        {
            auto polys = intersection(record->poly, roi_bounding_box);
            auto range = unique_ptr<Range>(new Range());
            for (const auto &poly : polys)
            {
                auto eids = discretizer.discretize(poly, false);
                for (auto eid : eids)
                {
                    if (intersects(discretizer.get_cell_polygon(eid), roi->poly))
                    {
                        range->elements.insert(Element{eid, 1});
                    }
                }
            }
            if (range->elements.size() > 0)
            {
                range->entity = record;
                ranges.push_back(move(range));
            }
        }
    }
}

struct Cell
{
    Polygon poly;
    list<Range *> owners;
};

Continuous::Continuous(Entity *roi, std::list<Entity *> records, double delta)
{
    ranges = func::map(records, [](Entity *record) {
        auto ret = unique_ptr<Range>(new Range());
        ret->entity = record;
        return ret;
    });

    auto cells = list<Cell>();

    for (const auto &range : ranges)
    {
        auto range_polys = intersection(range->entity->poly, roi->poly);
        auto new_inner_cells = list<Cell>();
        auto new_outer_cells = list<Cell>();
        for (const auto &range_poly : range_polys)
        {
            for (auto it = cells.begin(); it != cells.end();)
            {
                auto inners = list<Polygon>();
                auto outers = list<Polygon>();
                tie(inners, outers) = clip(it->poly, range_poly);
                if (inners.size() > 0)
                {
                    auto prev_owners = move(it->owners);
                    it = cells.erase(it);
                    for (auto outer : outers)
                    {
                        new_outer_cells.push_back(Cell{outer, prev_owners});
                    }
                    prev_owners.push_back(range.get());
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
        range_polys = difference(range_polys,
                                 func::map(new_inner_cells,
                                           [](const Cell &cell) {
                                               return cell.poly;
                                           }));

        for (const auto &range_poly : range_polys)
        {
            new_outer_cells.push_back(Cell{range_poly, {range.get()}});
        }

        cells.splice(cells.end(), new_inner_cells);
        cells.splice(cells.end(), new_outer_cells);
    }

    auto area_map = map<list<Range *>, double>();
    auto debug_counter = map<list<Range *>, double>();

    for (const auto &cell : cells)
    {
        if (area_map.count(cell.owners) == 0)
        {
            area_map[cell.owners] = 0;
            debug_counter[cell.owners] = 0;
        }
        area_map[cell.owners] += area(cell.poly);
        debug_counter[cell.owners] += 1;
    }

    cout << func::mean(debug_counter, [](const decltype(debug_counter)::value_type & kv){
        return kv.second;
    }) << endl;

    // this is to verify whether the cell covers the whole area 
    // double debug_area = 0;

    universe = unique_ptr<Range>(new Range());
    {
        int cell_index = 0; // index for cell
        for (const auto &kv : area_map)
        {
            // update universe
            universe->elements.insert(Element{cell_index, kv.second});

            // update ranges
            for (auto owner : kv.first)
            {
                owner->elements.insert(Element{cell_index, kv.second});
            }

            ++cell_index;

            // debug_area += kv.second;
        }
    }

    // cout << debug_area << " " << area(roi->poly) << " " <<  debug_area / area(roi->poly) << endl;
}

// void remove_scenes_with_no_cells(list<Scene *> &scenes)
// {
//     scenes.erase(remove_if(scenes.begin(),
//                            scenes.end(),
//                            [](const Scene *scene) {
//                                return scene->cell_set.size() == 0;
//                            }),
//                  scenes.end());
// }

// void remove_tiny_offcuts(list<Polygon> &offcuts, double delta)
// {
//     double area_threshold = delta * delta;
//     offcuts.erase(remove_if(offcuts.begin(),
//                             offcuts.end(),
//                             [area_threshold](const Polygon &offcut) {
//                                 return area(offcut) < area_threshold;
//                             }),
//                   offcuts.end());
// }

// void remove_scenes_with_no_offcuts(list<Scene *> &scenes)
// {
//     scenes.erase(remove_if(scenes.begin(),
//                            scenes.end(),
//                            [](const Scene *scene) {
//                                return scene->offcuts.size() == 0;
//                            }),
//                  scenes.end());
// }

// nlohmann::json to_json(const list<Polygon> &polys)
// {
//     auto ret = nlohmann::json();
//     for (const auto &poly : polys)
//     {
//         ret.push_back(to_string(poly));
//     }
//     return ret;
// }

// double area(const list<Polygon> &offcuts)
// {
//     double ret = 0;
//     for (const auto &offcut : offcuts)
//     {
//         ret += area(offcut);
//     }
//     return ret;
// };

// double calculate_coverage_ratio(ROI *roi, const list<Scene *> &scenes)
// {
//     double covered = 0;
//     continuous::cut_roi(roi);
//     for (auto i = scenes.begin(); i != scenes.end(); ++i)
//     {
//         continuous::cut_scenes(scenes, roi);
//     }
//     for (auto i = scenes.begin(); i != scenes.end(); ++i)
//     {
//         covered += area((*i)->offcuts);
//         for (auto j = next(i); j != scenes.end(); ++j)
//         {
//             (*j)->offcuts = difference((*j)->offcuts, (*i)->offcuts);
//         }
//     }
//     return covered / area(roi->poly);
// }

// double area(const CellRange &cell_set)
// {
//     return func::sum(cell_set, [](const int &cid) {
//         return Scene::area_map[cid];
//     });
// }