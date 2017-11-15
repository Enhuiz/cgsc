#include "model.h"

#include <set>
#include <iostream>
#include <fstream>

using namespace std;

namespace discrete
{
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

    Polygon get_cell_polygon(const CID &cid) const
    {
        return get_cell_polygon(cid_to_point(cid));
    }

    CellSet discretize(const Polygon &poly, bool keep_edge_cells) const
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

        CellSet ret;
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

    CID index_to_cid(int xi, int yi) const
    {
        return xi + ((CID)yi << 32);
    }

    Point cid_to_point(const CID &cid) const
    {
        int xi = cid & (~0);
        int yi = cid >> 32;
        return {xi * delta, yi * delta};
    }
};

void discretize_roi(ROI *roi, double delta)
{
    Discretizer discretizer{delta};
    roi->cell_set = discretizer.discretize(roi->poly, true);
}

void discretize_scenes(const list<Scene *> &scenes, ROI *roi, double delta)
{
    Discretizer discretizer{delta};
    auto roi_bbox = discretizer.bounding_box(roi->poly, floor_double, ceil_double);
    for (auto scene : scenes) // n
    {
        scene->cell_set.clear();
        auto polys = intersection(roi_bbox, scene->poly); // this indeed speed up the next instruction
        for (const auto &poly : polys)
        {
            auto cell_set = discretizer.discretize(poly, false);
            // find cid in the intersections
            for (auto cid : cell_set) // O(m)
            {
                if (intersects(discretizer.get_cell_polygon(cid), roi->poly)) // was O(logM), is O(1)
                {
                    scene->cell_set.insert(cid); // O(logm)
                }
            }
        }
    }
}

void transform(ROI *roi, list<Scene *> &scenes, double delta)
{
    discretize_roi(roi, delta);
    discretize_scenes(scenes, roi, delta);
    remove_scenes_with_no_cells(scenes);
}
}

namespace continuous
{

void cut_roi(ROI *roi)
{
    if (convex(roi->poly))
    {
        roi->offcuts = list<Polygon>{roi->poly};
    }
    else
    {
        roi->offcuts = triangulate(roi->poly);
    }
}

void cut_scenes(const list<Scene *> &scenes, ROI *roi)
{
    for (auto scene : scenes)
    {
        if (convex(scene->poly))
        {
            scene->offcuts = list<Polygon>{scene->poly};
        }
        else
        {
            scene->offcuts = triangulate(scene->poly);
        }
        scene->offcuts = intersection(scene->offcuts, roi->offcuts);
    }
}

void transform(ROI *roi, list<Scene *> &scenes, double delta)
{
    cut_roi(roi);
    remove_tiny_offcuts(roi->offcuts, delta);
    cut_scenes(scenes, roi);
    for (auto scene : scenes)
    {
        remove_tiny_offcuts(scene->offcuts, delta);
    }
    remove_scenes_with_no_offcuts(scenes);
}
}

namespace semantical
{
struct Cell
{
    Polygon poly;
    list<int> owners;
};

vector<double> transform(ROI *roi, list<Scene *> &scenes, double delta)
{
    auto indexed_scenes = vector<Scene *>(scenes.begin(), scenes.end());
    auto cells = list<Cell>();

    for (int i = 0; i < indexed_scenes.size(); ++i)
    {
        auto scene_offcuts = intersection(indexed_scenes[i]->poly, roi->poly);
        auto new_inner_cells = list<Cell>();
        auto new_outer_cells = list<Cell>();
        for (const auto& scene_offcut : scene_offcuts)
        {
            for (auto it = cells.begin(); it != cells.end();)
            {
                auto inners = list<Polygon>();
                auto outers = list<Polygon>();
                tie(inners, outers) = clip(it->poly, scene_offcut);
                if (inners.size() > 0)
                {
                    auto prev_owners = move(it->owners);
                    it = cells.erase(it);
                    for (auto outer : outers)
                    {
                        if (area(outer) > delta * delta)
                        {
                            new_outer_cells.push_back(Cell{outer, prev_owners});
                        }
                    }
                    prev_owners.push_back(i);
                    for (auto inner : inners)
                    {
                        double inner_area = area(inner);
                        if (inner_area > delta * delta)
                        {
                            new_inner_cells.push_back(Cell{inner, prev_owners});
                        }
                    }
                }
                else
                {
                    ++it;
                }
            }
        }
        // we need to implement new method to calculate offcuts
        // or we can just do this
        {
            scene_offcuts = difference(scene_offcuts,
                                       func::map(new_inner_cells,
                                                 [](const Cell &cell) {
                                                     return cell.poly;
                                                 }));

            for (const auto& scene_offcut: scene_offcuts)
            {
                if (area(scene_offcut) > delta * delta)
                {
                    new_outer_cells.push_back(Cell{scene_offcut, {i}});
                }
            }
        }

        cells.splice(cells.end(), new_inner_cells);
        cells.splice(cells.end(), new_outer_cells);
    }

    auto cell_map = map<list<int>, list<Cell>>();
 
    for (const auto& cell: cells)
    {
        if (cell_map.count(cell.owners) == 0)
        {
            cell_map[cell.owners] = {};
        }
        cell_map[cell.owners].push_back(cell);
    }

    auto area_table = vector<double>();
    {
        int cell_index = 0;
        area_table.resize(cells.size());
        for (const auto& kv: cell_map)
        {
            // update roi
            roi->cell_set.insert(cell_index);

            // update cell
            for (auto owner : kv.first)
            {
                auto scene = cell_indexed_scenes[owner];
                scene->cell_set.insert(cell_index);
            }

            // update area table
            area_table[cell_index] = func::sum(kv.second, [](const Cell &cell) {
                return area(cell.poly);
            });
            //
            ++cell_index;
        }
    }

    return area_table;
}
}

void remove_scenes_with_no_cells(list<Scene *> &scenes)
{
    scenes.erase(remove_if(scenes.begin(),
                           scenes.end(),
                           [](const Scene *scene) {
                               return scene->cell_set.size() == 0;
                           }),
                 scenes.end());
}

void remove_tiny_offcuts(list<Polygon> &offcuts, double delta)
{
    double area_threshold = delta * delta;
    offcuts.erase(remove_if(offcuts.begin(),
                            offcuts.end(),
                            [area_threshold](const Polygon &offcut) {
                                return area(offcut) < area_threshold;
                            }),
                  offcuts.end());
}

void remove_scenes_with_no_offcuts(list<Scene *> &scenes)
{
    scenes.erase(remove_if(scenes.begin(),
                           scenes.end(),
                           [](const Scene *scene) {
                               return scene->offcuts.size() == 0;
                           }),
                 scenes.end());
}

nlohmann::json to_json(const list<Polygon> &polys)
{
    auto ret = nlohmann::json();
    for (const auto &poly : polys)
    {
        ret.push_back(to_string(poly));
    }
    return ret;
}

double area(const list<Polygon> &offcuts)
{
    double ret = 0;
    for (const auto &offcut : offcuts)
    {
        ret += area(offcut);
    }
    return ret;
};

double calculate_coverage_ratio(ROI *roi, const list<Scene *> &scenes)
{
    double covered = 0;
    continuous::cut_roi(roi);
    for (auto i = scenes.begin(); i != scenes.end(); ++i)
    {
        continuous::cut_scenes(scenes, roi);
    }
    for (auto i = scenes.begin(); i != scenes.end(); ++i)
    {
        covered += area((*i)->offcuts);
        for (auto j = next(i); j != scenes.end(); ++j)
        {
            (*j)->offcuts = difference((*j)->offcuts, (*i)->offcuts);
        }
    }
    return covered / area(roi->poly);
}

double area(const CellSet &cell_set, const std::vector<double> &area_table)
{
    return func::sum(cell_set, [&area_table](const CID &cid) {
        return area_table[cid];
    });
}