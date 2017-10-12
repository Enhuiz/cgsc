#include "solution.h"

#include <algorithm>
#include <list>
#include <iostream>

using namespace std;

vector<Scene *> select_possible_scenes(AOI *aoi, const vector<Scene *> &scenes)
{
    vector<Scene *> ret;
    copy_if(scenes.begin(), scenes.end(), back_inserter(ret), [aoi](Scene *scene) {
        return intersects(aoi->poly, scene->poly);
    });
    return ret;
}

namespace discrete
{

void discretize_aoi(const Discretizer &discretizer, AOI *aoi)
{
    aoi->cell_set = discretizer.discretize(aoi->poly, true);
}

// O(nmlogm)
void discretize_scenes(const Discretizer &discretizer, AOI *aoi, const vector<Scene *> &scenes)
{
    auto &acs = aoi->cell_set;
    for (auto scene : scenes) // n
    {
        scene->cell_set = discretizer.discretize(scene->poly, false); //mlogm

        // intersection
        CellSet new_cs;
        auto &cs = scene->cell_set;
        set_intersection(cs.begin(), cs.end(), acs.begin(), acs.end(), inserter(new_cs, new_cs.begin()));
        cs = new_cs;
    }
}

// O(n^2mlogm)
vector<Scene *> select_approx_optimal_scenes(AOI *aoi, const vector<Scene *> &scenes)
{
    list<Scene *> possible_scenes(scenes.begin(), scenes.end());
    vector<Scene *> result_scenes;
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
                possible_scene->cell_set.erase(cell); // logm
            }
        }
        covered += scene->cell_set.size();
        result_scenes.push_back(scene);
        remove_scenes_with_empty_cell_set(possible_scenes);
    }
    return result_scenes;
}
}

namespace continuous
{

vector<Scene *> select_approx_optimal_scenes(AOI *aoi, const vector<Scene *> &scenes)
{
    auto scene_different_boost_polygon = [](Scene *scene, const BoostPolygon& bpoly) {
        vector<BoostPolygon> result_bpolys;
        for (const auto &sbpoly : scene->bpolys)
        {
            vector<BoostPolygon> output_bpolys;
            boost::geometry::difference(sbpoly, bpoly, output_bpolys);
            result_bpolys.insert(result_bpolys.end(), output_bpolys.begin(), output_bpolys.end());
        }
        scene->bpolys = result_bpolys;
    };

    auto scene_different_boost_polygons = [scene_different_boost_polygon](Scene *scene, vector<BoostPolygon> &bpolys) {
        for (auto bpoly : bpolys)
        {
            scene_different_boost_polygon(scene, bpoly);
        }
    };

    auto point_to_boost_point = [](const Point &p) {
        return BoostPoint(p.x, p.y);
    };

    auto polygon_to_boost_polygons = [point_to_boost_point](const Polygon &poly) {
        vector<BoostPoint> bps;
        bps.reserve(poly.size());
        for (const auto &p : poly)
        {
            bps.push_back(point_to_boost_point(p));
        }
        BoostPolygon bpoly;
        boost::geometry::append(bpoly, bps);
        return vector<BoostPolygon>{bpoly};
    };

    { // assemble bpoly
        aoi->bpolys = polygon_to_boost_polygons(aoi->poly);
        for (auto scene : scenes)
        {
            scene->bpolys = polygon_to_boost_polygons(scene->poly);
            scene_different_boost_polygons(scene, aoi->bpolys);
        }
    }

    vector<BoostPolygon> covered;
    list<Scene *> possible_scenes(scenes.begin(), scenes.end());
    vector<Scene *> result_scenes;

    auto covered_area = [&covered]() {
        double ret = 0;
        for (auto bpoly : covered)
        {
            ret += boost::geometry::area(bpoly);
        }
        return ret;
    };

    auto scene_area = [](const Scene *scene) {
        double ret = 0;
        for (const auto &bpoly : scene->bpolys)
        {
            ret += boost::geometry::area(bpoly);
        }
        return ret;
    };

    double aoi_area = 0;
    for (const auto& bpoly: aoi->bpolys)
        aoi_area += boost::geometry::area(bpoly);

    auto remove_scenes_with_empty_bpolys = [](list<Scene *> scenes) {
        scenes.erase(remove_if(scenes.begin(),
                               scenes.end(),
                               [](const Scene *scene) {
                                   return scene->bpolys.size() == 0;
                               }),
                     scenes.end());
    };

    while (covered_area() < aoi_area && possible_scenes.size() > 0) // n
    {
        auto it = min_element(possible_scenes.begin(), possible_scenes.end(), [scene_area](const Scene *a, const Scene *b) { // n
            return a->price / scene_area(a) < b->price / scene_area(b);
        });
        // remove the selected
        auto scene = *it;
        possible_scenes.erase(it);
        // remove bpoly from the left possible scenes
        for (auto possible_scene : possible_scenes) // n
        {
            scene_different_boost_polygons(possible_scene, scene->bpolys);
        }
        covered.insert(covered.end(), scene->bpolys.begin(), scene->bpolys.end());
        result_scenes.push_back(scene);
        remove_scenes_with_empty_bpolys(possible_scenes);
    }

    return result_scenes;
}
}