#include "solution.h"

#include <algorithm>
#include <list>
#include <iostream>
#include <functional>
#include <fstream>

#include "global.h"

using namespace std;

list<Scene *> select_possible_scenes(ROI *roi, const list<Scene *> &scenes)
{
    list<Scene *> ret;
    for (auto scene : scenes)
    {
        if (intersects(scene->poly, roi->poly))
        {
            ret.push_back(scene);
        }
    }
    return ret;
}

namespace discrete
{
namespace greedy
{

list<Scene *> optimize(ROI *roi, list<Scene *> possible_scenes, double delta)
{
    Stopwatch sw;

    sw.restart();
    {
        discretize_roi(roi, delta);
        discretize_scenes(possible_scenes, roi, delta);
    }
    g_report["t1"] = sw.lap();

    sw.restart();
    auto selected_scenes = list<Scene *>();
    {
        int covered = 0;
        int to_cover = roi->cell_set.size();
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
        roi->cell_set.clear();
    }
    g_report["t2"] = sw.lap();

    return selected_scenes;
}
}
}

namespace continuous
{
namespace greedy
{
list<Scene *> optimize(ROI *roi, list<Scene *> possible_scenes, double delta)
{
    Stopwatch sw;

    sw.restart();
    {
        cut_roi(roi);
        remove_tiny_offcuts(roi->offcuts, delta);
        cut_scenes(possible_scenes, roi);
        for (auto possible_scene : possible_scenes)
        {
            remove_tiny_offcuts(possible_scene->offcuts, delta);
        }
        remove_scenes_with_no_offcuts(possible_scenes);
    }
    g_report["t1"] = sw.lap();

    g_report["iterations"] = {
        {"coverage_ratio", {}},
        {"price", {}},
        {"time", {}},
    };

    sw.restart();
    auto selected_scenes = list<Scene *>();
    {
        double covered = 0;
        double to_cover = area(roi->offcuts);
        double price = 0;

        while (covered < to_cover && !almost_equal(to_cover, covered, 1e3) && possible_scenes.size() > 0) // n
        {
            // select current
            // Stopwatch sw;
            // sw.restart();
            auto it = min_element(possible_scenes.begin(), possible_scenes.end(), [](const Scene *a, const Scene *b) { // n
                return a->price / area(a->offcuts) < b->price / area(b->offcuts);
            });
            // g_report["t2.1"] = sw.lap();

            // claim the selected
            // sw.restart();
            auto selected_scene = *it;
            possible_scenes.erase(it);
            selected_scenes.push_back(selected_scene);
            covered += area(selected_scene->offcuts);
            // g_report["iterations"]["coverage_ratio"].push_back(covered / to_cover);
            // price += selected_scene->price;
            // g_report["iterations"]["price"].push_back(price);
            // g_report["iterations"]["time"].push_back(sw.lap());
            // g_report["t2.2"] = sw.lap();

            // update left
            // sw.restart();
            for (auto possible_scene : possible_scenes) // n
            {
                if (!intersects(possible_scene->poly, selected_scene->poly))
                    continue;
                possible_scene->offcuts = difference(possible_scene->offcuts, selected_scene->offcuts); // m * m
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
            selected_scene->offcuts.clear();
        }
        roi->offcuts.clear();
    }
    g_report["t2"] = sw.lap();

    return selected_scenes;
}
}

namespace branch_and_bound
{
struct State
{
    double price;
    double covered;
    list<Scene *> selected;
};

void remove_scenes_disjoint_with(list<Scene *> &scenes, const list<Polygon> &polys)
{
    scenes.erase(remove_if(scenes.begin(),
                           scenes.end(),
                           [&polys](const Scene *scene) {
                               return none_of(polys.begin(), polys.end(), [scene](const Polygon &poly) {
                                   return intersects(scene->poly, poly);
                               });
                           }),
                 scenes.end());
}

State optimistic_estimate(State state, ROI *roi, list<Scene *> possible_scenes)
{
    possible_scenes.sort([](Scene *a, Scene *b) {
        return a->price / a->valid_area < b->price / b->valid_area;
    });

    for (const auto &offcut : roi->offcuts)
    {
        auto pieces = list<Polygon>{offcut}; // roi pieces
        for (auto possible_scene : possible_scenes)
        {
            for (const auto scene_offcut : possible_scene->offcuts)
            {
                for (auto it = pieces.begin(); it != pieces.end();)
                {
                    const auto &piece = *it;
                    auto inners = list<Polygon>();
                    auto outers = list<Polygon>();
                    tie(inners, outers) = clip(piece, scene_offcut);
                    if (inners.size() > 0) // intersects
                    {
                        for (const auto &inner : inners)
                        {
                            double inner_area = area(inner);
                            state.covered += inner_area;
                            state.price += possible_scene->price / possible_scene->valid_area * inner_area;
                        }
                        for (const auto &outer : outers)
                        {
                            pieces.push_back(outer);
                        }
                        it = pieces.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }
            }
        }
    }
    return state;
}

list<Scene *> optimize(ROI *roi, list<Scene *> possible_scenes, double delta)
{
    Stopwatch sw;

    sw.restart();
    {
        continuous::cut_roi(roi);
        continuous::remove_tiny_offcuts(roi->offcuts, delta);
        continuous::cut_scenes(possible_scenes, roi);
        for (auto possible_scene : possible_scenes)
        {
            continuous::remove_tiny_offcuts(possible_scene->offcuts, delta);
        }
        continuous::remove_scenes_with_no_offcuts(possible_scenes);
    }
    g_report["t1"] = sw.lap();

    logger.debug(to_string(possible_scenes.size()));

    auto optimal_state = State{numeric_limits<double>::max(), 0, list<Scene *>()};
    sw.restart();
    {
        double target_area = 0.95 * area(roi->poly);

        const function<void(const State &, list<Scene *>)> helper =
            [&helper, roi, target_area, delta, &optimal_state](const State &prev_state, list<Scene *> possible_scenes) {
                remove_scenes_disjoint_with(possible_scenes, roi->offcuts);

                auto optimistic_state = optimistic_estimate(prev_state, roi, possible_scenes);
                if (optimistic_state.covered < target_area || optimistic_state.price >= optimal_state.price)
                {
                    return;
                }

                if (possible_scenes.size() == 0)
                {
                    optimal_state = prev_state;
                    logger << optimal_state.price << endl;
                    logger << optimal_state.covered / target_area * 0.95 << endl;
                    return;
                }

                auto current_scene = possible_scenes.front();
                possible_scenes.pop_front();

                { // skip current scene
                    const auto &state = prev_state;
                    helper(state, possible_scenes);
                }

                { // select current scene
                    auto state = prev_state;
                    auto prev_offcuts = roi->offcuts;
                    roi->offcuts = difference(roi->offcuts, current_scene->offcuts);
                    continuous::remove_tiny_offcuts(roi->offcuts, delta);
                    state.covered += continuous::area(prev_offcuts) - continuous::area(roi->offcuts);
                    state.price += current_scene->price;
                    state.selected.push_back(current_scene);
                    helper(state, possible_scenes);
                    roi->offcuts = prev_offcuts;
                }
            };
        helper(State{0, 0, list<Scene *>()}, possible_scenes);
    }
    g_report["t2"] = sw.lap();

    return optimal_state.selected;
}
}
}
