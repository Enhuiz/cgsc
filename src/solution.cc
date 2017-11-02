#include "solution.h"

#include <algorithm>
#include <list>
#include <iostream>
#include <functional>
#include <fstream>

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

void remove_scenes_with_empty_cell_set(list<Scene *> &scenes)
{
    scenes.erase(remove_if(scenes.begin(),
                           scenes.end(),
                           [](const Scene *scene) {
                               return scene->cell_set.size() == 0;
                           }),
                 scenes.end());
};

list<Scene *> select_approx_optimal_scenes(AOI *aoi, list<Scene *> possible_scenes, double delta)
{
    Stopwatch sw;

    sw.restart();
    {
        discretize_aoi(aoi, delta);
        discretize_scenes(possible_scenes, aoi, delta);
    }
    g_report["t1"] = sw.lap();

    sw.restart();
    auto selected_scenes = list<Scene *>();
    {
        int covered = 0;
        int to_cover = aoi->cell_set.size();
        remove_scenes_with_empty_cell_set(possible_scenes);
        while (covered < to_cover && possible_scenes.size() > 0) // n
        {
            // select current
            // sw.restart();
            auto it = min_element(possible_scenes.begin(), possible_scenes.end(), [](const Scene *a, const Scene *b) { // n
                return a->price / a->cell_set.size() < b->price / b->cell_set.size();
            });
            // g_report["t2.1"] = sw.lap();

            // claim the selected
            // sw.restart();
            auto selected_scene = *it;
            possible_scenes.erase(it);
            covered += selected_scene->cell_set.size();
            selected_scenes.push_back(selected_scene);
            // g_report["t2.2"] = sw.lap();

            // update rest
            // remove cells from the left possible scenes
            // sw.restart();
            {
                const auto &scs = selected_scene->cell_set;
                for (auto &possible_scene : possible_scenes) // n
                {
                    if (!intersects(possible_scene->poly, selected_scene->poly))
                        continue;
                    // auto &pcs = possible_scene->cell_set;
                    // CellSet tmp;
                    // set_difference(pcs.begin(), pcs.end(), scs.begin(), scs.end(), inserter(tmp, tmp.begin())); // max(m, m);
                    // pcs = move(tmp);
                    for (const auto &cell : selected_scene->cell_set) // m
                    {
                        possible_scene->cell_set.erase(cell); // 1
                    }
                }
            }
            // g_report["t2.3"] = sw.lap();

            // sw.restart();
            remove_scenes_with_empty_cell_set(possible_scenes);
            // g_report["t2.4"] = sw.lap();

            // visualization
            // cerr << covered * 100.0 / to_cover << "%: " << possible_scenes.size() << endl;
        }

        // release memory
        for (auto possible_scene : possible_scenes)
        {
            possible_scene->cell_set.clear();
        }
        for (auto selected_scene : selected_scenes)
        {
            selected_scene->cell_set.clear();
        }
        aoi->cell_set.clear();
    }
    g_report["t2"] = sw.lap();

    return selected_scenes;
}
}

namespace continuous
{

void remove_tiny_offcuts(list<Polygon> &offcuts, double delta)
{
    offcuts.erase(remove_if(offcuts.begin(),
                            offcuts.end(),
                            [delta](const Polygon &offcut) {
                                return area(offcut) < delta * delta;
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
};

// void triangulate_non_convex_offcuts(list<Polygon> &offcuts, double delta)
// {
//     for (auto it = offcuts.begin(); it != offcuts.end();)
//     {
//         if (!convex(*it))
//         {
//             // logger.debug("very unlikely to happen");
//             auto triangles = triangulate(*it);
//             remove_tiny_offcuts(triangles, delta);
//             offcuts.splice(offcuts.end(), triangles);
//             it = offcuts.erase(it);
//         }
//         else
//         {
//             ++it;
//         }
//     }
// }


list<Scene *> select_approx_optimal_scenes(AOI *aoi, list<Scene *> possible_scenes, double delta)
{
    Stopwatch sw;

    sw.restart();
    {
        cut_aoi(aoi);
        remove_tiny_offcuts(aoi->offcuts, delta);
        cut_scenes(possible_scenes, aoi);
        for (auto possible_scene : possible_scenes)
        {
            remove_tiny_offcuts(possible_scene->offcuts, delta);
        }
        remove_scenes_with_no_offcuts(possible_scenes);
    }
    g_report["t1"] = sw.lap();

    sw.restart();
    auto selected_scenes = list<Scene *>();
    {
        double covered = 0;
        double to_cover = area(aoi->offcuts);

        while (covered < to_cover && !almost_equal(to_cover, covered, 1e3) && possible_scenes.size() > 0) // n
        {
            // select current
            // sw.restart();
            auto it = min_element(possible_scenes.begin(), possible_scenes.end(), [](const Scene *a, const Scene *b) { // n
                return a->price / area(a->offcuts) < b->price / area(b->offcuts);
            });
            // g_report["t2.1"] = sw.lap();

            // claim the selected
            // sw.restart();
            auto scene = *it;
            possible_scenes.erase(it);
            selected_scenes.push_back(scene);
            covered += area(scene->offcuts);
            // g_report["t2.2"] = sw.lap();

            // update left
            // sw.restart();
            for (auto possible_scene : possible_scenes) // n
            {
                if (!intersects(possible_scene->poly, scene->poly))
                    continue;
                possible_scene->offcuts = difference(possible_scene->offcuts, scene->offcuts); // m * m
                remove_tiny_offcuts(possible_scene->offcuts, delta);
            }
            // g_report["t2.3"] = sw.lap();

            // remove empty
            // sw.restart();
            remove_scenes_with_no_offcuts(possible_scenes);
            // g_report["t2.4"] = sw.lap();

            // cerr << covered / to_cover * 100 << "\%; " << possible_scenes.size() << endl;
        }

        // release memory
        for (auto possible_scene : possible_scenes)
        {
            possible_scene->offcuts.clear();
        }
        for (auto selected_scene : selected_scenes)
        {
            selected_scene->cell_set.clear();
        }
        aoi->offcuts.clear();
    }
    g_report["t2"] = sw.lap();

    return selected_scenes;
}
}

namespace brute_force
{
list<Scene *> select_optimal_scenes(AOI *aoi, const list<Scene *> &scenes, double delta)
{
    auto scene = scenes.front();
    auto new_aoi = unique_ptr<AOI>(new AOI);
}
}
