#include "solution.h"

#include <algorithm>
#include <list>
#include <iostream>
#include <functional>

#include "global.h"

using namespace std;

list<Scene *> select_possible_scenes(AOI *aoi, const list<Scene *> &scenes)
{
    list<Scene *> ret;
    for (auto scene : scenes)
    {
        if (intersects(scene->poly, aoi->poly))
        {
            ret.push_back(scene);
        }
    }
    return ret;
}

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

void discretize_aoi(AOI *aoi, double delta)
{
    Discretizer discretizer{delta};
    aoi->cell_set = discretizer.discretize(aoi->poly, true);
}

// O(n m log m)
void discretize_scenes(const std::list<Scene *> &scenes, AOI *aoi, double delta)
{
    Discretizer discretizer{delta};
    auto aoi_bbox = discretizer.bounding_box(aoi->poly, floor_double, ceil_double);
    for (auto scene : scenes) // n
    {
        auto poly = intersection(aoi_bbox, scene->poly);
        auto cell_set = discretizer.discretize(poly, false);
        // find cid in the intersections
        for (const auto &cid : cell_set) // O(m)
        {
            if (intersects(discretizer.get_cell_polygon(cid), aoi->poly))
            {
                scene->cell_set.insert(cid); // O(logm)
            }
        }
    }
}

// O(n^2 m log m)
list<Scene *> select_approx_optimal_scenes(AOI *aoi, const list<Scene *> &scenes)
{
    list<Scene *> possible_scenes(scenes.begin(), scenes.end());
    list<Scene *> result_scenes;
    int covered = 0;
    int num_aoi_cells = aoi->cell_set.size();

    auto remove_scenes_with_empty_cell_set = [](list<Scene *> &scenes) {
        scenes.erase(remove_if(scenes.begin(),
                               scenes.end(),
                               [](const Scene *scene) {
                                   return scene->cell_set.size() == 0;
                               }),
                     scenes.end());
    };

    remove_scenes_with_empty_cell_set(possible_scenes);
    while (covered < num_aoi_cells && possible_scenes.size() > 0) // n
    {
        auto it = min_element(possible_scenes.begin(), possible_scenes.end(), [](const Scene *a, const Scene *b) { // n
            return a->price / a->cell_set.size() < b->price / b->cell_set.size();
        });
        // remove the selected
        auto scene = *it;
        possible_scenes.erase(it);
        // remove cells from the left possible scenes
        for (auto &possible_scene : possible_scenes) // n
        {
            for (const auto &cell : scene->cell_set) // m
            {
                possible_scene->cell_set.erase(cell); // log m
            }
        }
        covered += scene->cell_set.size();
        result_scenes.push_back(scene);
        remove_scenes_with_empty_cell_set(possible_scenes);
        // cerr << covered * 100.0 / num_aoi_cells << "%: " << possible_scenes.size() << endl;
    }
    return result_scenes;
}
}

namespace continuous
{

struct Cutter
{
    double delta;
    void remove_tiny_offcuts(list<Polygon> &offcuts)
    {
        offcuts.erase(remove_if(offcuts.begin(),
                                offcuts.end(),
                                [this](const Polygon &offcut) {
                                    return area(offcut) < delta * delta;
                                }),
                      offcuts.end());
    }

    void scene_op_offcut(Scene *scene, const Polygon &offcut, function<list<Polygon>(const Polygon &, const Polygon &)> op)
    {
        list<Polygon> results;
        for (const auto &scene_offcut : scene->offcuts)
        {
            results.splice(results.end(), op(scene_offcut, offcut));
        }
        scene->offcuts = results;
        remove_tiny_offcuts(scene->offcuts);
    }

    void scene_op_offcuts(Scene *scene, const list<Polygon> &offcuts, function<list<Polygon>(const Polygon &, const Polygon &)> op)
    {
        for (const auto &offcut : offcuts)
        {
            scene_op_offcut(scene, offcut, op);
        }
    }

    void scene_difference_offcuts(Scene *scene, const list<Polygon> &offcuts)
    {
        scene_op_offcuts(scene, offcuts, difference);
    }

    void scene_intersection_offcuts(Scene *scene, const list<Polygon> &offcuts)
    {
        scene_op_offcuts(scene, offcuts, [](const Polygon &a, const Polygon &b) {
            return list<Polygon>{intersection(a, b)};
        });
    }
};

void cut_aoi(AOI *aoi, double delta)
{
    Cutter cutter{delta};
    if (convex(aoi->poly))
    {
        aoi->offcuts = list<Polygon>{aoi->poly};
    }
    else
    {
        // aoi->offcuts = triangulate(aoi->poly);
    }
    cutter.remove_tiny_offcuts(aoi->offcuts);
}

void cut_scenes(const list<Scene *> &scenes, AOI *aoi, double delta)
{
    Cutter cutter{delta};
    for (auto scene : scenes)
    {
        if (convex(scene->poly))
        {
            scene->offcuts = list<Polygon>{scene->poly};
        }
        else
        {
            // scene->offcuts = scene->poly;
        }
        cutter.scene_intersection_offcuts(scene, aoi->offcuts);
    }
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

double area(const Scene *scene)
{
    return area(scene->offcuts);
};

double area(const AOI *aoi)
{
    return area(aoi->offcuts);
};

void remove_scenes_with_no_offcuts(list<Scene *> &scenes)
{
    scenes.erase(remove_if(scenes.begin(),
                           scenes.end(),
                           [](const Scene *scene) {
                               return scene->offcuts.size() == 0;
                           }),
                 scenes.end());
};

list<Scene *> select_approx_optimal_scenes(AOI *aoi, const list<Scene *> &scenes, double delta)
{
    Cutter cutter{delta};
    list<Scene *> possible_scenes(scenes.begin(), scenes.end());
    list<Scene *> result_scenes;
    double covered_area = 0;
    double aoi_area = area(aoi);

    remove_scenes_with_no_offcuts(possible_scenes);
    while (covered_area < aoi_area && !almost_equal(aoi_area, covered_area, 10) && possible_scenes.size() > 0) // n
    {
        auto it = min_element(possible_scenes.begin(), possible_scenes.end(), [](const Scene *a, const Scene *b) { // n
            return a->price / area(a) < b->price / area(b);
        });
        // remove the selected
        auto scene = *it;
        possible_scenes.erase(it);
        // add covered area
        covered_area += area(scene);
        // clip the rest possible scenes
        for (auto possible_scene : possible_scenes) // n
        {
            cutter.scene_difference_offcuts(possible_scene, scene->offcuts);
        }
        remove_scenes_with_no_offcuts(possible_scenes);
        result_scenes.push_back(scene);
        // cerr << covered_area / aoi_area * 100 << "\%; " << possible_scenes.size() << endl;
    }
    return result_scenes;
}
}
