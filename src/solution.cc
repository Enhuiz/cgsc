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

list<Scene *> optimize(ROI *roi, list<Scene *> possible_scenes, double target_coverage_ratio, double delta)
{
    Stopwatch sw;

    sw.restart();
    {
        discretize_roi(roi, delta);
        discretize_scenes(possible_scenes, roi, delta);
    }
    g_report["t1"] = sw.lap();

    sw.restart();
    int covered = 0;
    int to_cover = roi->cell_set.size() * target_coverage_ratio;
    auto selected_scenes = list<Scene *>();
    {
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

    if (covered < to_cover)
        return {};

    return selected_scenes;
}
}
}

namespace continuous
{
namespace greedy
{
list<Scene *> optimize(ROI *roi, list<Scene *> possible_scenes, double target_coverage_ratio, double delta)
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
    double covered = 0;
    double to_cover = area(roi->offcuts) * target_coverage_ratio;
    auto selected_scenes = list<Scene *>();
    {
        // double price = 0;
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

    if (covered < to_cover)
        return {};

    return selected_scenes;
}
}

namespace branch_and_bound
{
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

namespace DFS
{
struct State
{
    double price;
    double covered;
    list<Scene *> selected;
};

State optimistic_estimate(State state, ROI *roi, list<Scene *> possible_scenes)
{
    possible_scenes.sort([](Scene *a, Scene *b) {
        return a->price / a->valid_area < b->price / b->valid_area;
    });

    auto update_optimistic = [&state, &possible_scenes](const Polygon &offcut) {
        auto pieces = list<Polygon>{offcut};
        for (auto possible_scene : possible_scenes)
        {
            for (const auto &scene_offcut : possible_scene->offcuts)
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
                if (pieces.size() == 0)
                    return;
            }
        }
    };

    for (const auto &offcut : roi->offcuts)
    {
        update_optimistic(offcut);
    }

    return state;
}

list<Scene *> optimize(ROI *roi, list<Scene *> possible_scenes, double target_coverage_ratio, double delta)
{
    Stopwatch sw;
    
    sw.restart();
    double roi_area = area(roi->poly);
    double to_cover = roi_area * target_coverage_ratio;
    auto optimal_state = State{};
    {
        { // initialize optimal state
            auto selected = greedy::optimize(roi, possible_scenes, target_coverage_ratio, delta);
            optimal_state.price = accumulate(selected.begin(), selected.end(), 0.0, [](double acc, Scene *scene) {
                return acc + scene->price;
            });
            optimal_state.covered = to_cover;
            optimal_state.selected = move(selected);
        }
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
    
    sw.restart();
    {
        double roi_area = area(roi->poly);
        double to_cover = roi_area * target_coverage_ratio;

        const function<void(const State &, list<Scene *>)> helper =
            [&helper, roi, delta, to_cover, &optimal_state, roi_area](const State &prev_state, list<Scene *> possible_scenes) {
                if (prev_state.covered >= to_cover && prev_state.price < optimal_state.price)
                {
                    optimal_state = prev_state;
                    logger << "[better solution]" << endl;
                    logger << "price: " << optimal_state.price << endl;
                    logger << "covered: " << optimal_state.covered / roi_area << endl;
                    logger << "scenes " << optimal_state.selected.size() << endl;
                    return;
                }

                remove_scenes_disjoint_with(possible_scenes, roi->offcuts);
                auto optimistic_state = optimistic_estimate(prev_state, roi, possible_scenes);
                if (optimistic_state.price >= optimal_state.price)
                {
                    return;
                }

                if (possible_scenes.size() == 0)
                {
                    if (optimistic_state.covered >= to_cover)
                    {
                        optimal_state = prev_state;
                        logger << "[tail better solution]" << endl;
                        logger << "price: " << optimal_state.price << endl;
                        logger << "covered: " << optimal_state.covered / roi_area << endl;
                        logger << "scenes " << optimal_state.selected.size() << endl;
                    }
                    return;
                }

                auto it = possible_scenes.begin();
                // { // randomly select one
                //     default_random_engine gen;
                //     uniform_int_distribution<> dis(0, possible_scenes.size() - 1);
                //     int n = dis(gen);
                //     while (n--)
                //     {
                //         ++it;
                //     }
                // }
                auto current_scene = *it;
                possible_scenes.erase(it);

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

                { // skip current scene
                    const auto &state = prev_state;
                    helper(state, possible_scenes);
                }
            };
        helper(State{0, 0, list<Scene *>()}, possible_scenes);
    }
    g_report["t2"] = sw.lap();

    return optimal_state.selected;
}
}

namespace BFS
{
struct State
{
    double price;   // the price of selected scenes
    double covered; // only if the covered is satisfied can the state be accepted as an optimal state

    double price_lower_bound;   // if the lower bound higher than the optimal, kill it
    // double covered_upper_bound; // if upper bound no good than target then kill it

    list<Scene *> selected;
    list<Scene *> possible;
    list<Polygon> offcuts;

    list<shared_ptr<State>> branch(double delta)
    {
        
        list<shared_ptr<State>> ret;
        
        { // discard
            auto state = shared_ptr<State>(new State(*this));
            state->possible.pop_front();
            ret.push_back(state);
        }
        
        { // adopt
            auto state = shared_ptr<State>(new State(*this));
            auto current_scene = possible.front();
            state->possible.pop_front();
            state->selected.push_back(current_scene);
            state->offcuts = difference(offcuts, current_scene->offcuts);
            state->covered += continuous::area(offcuts) - continuous::area(state->offcuts);
            continuous::remove_tiny_offcuts(state->offcuts, delta);
            state->price += current_scene->price;
            ret.push_back(state);
        }

        return ret;
    }

    void print()
    {
        logger << "-----------state----------" << endl;
        logger << "price: " << price << endl;
        logger << "covered: " << covered << endl;
        logger << "price_lower_bound: " << price_lower_bound << endl;
        logger << "offcuts.size(): " << offcuts.size() << endl;
        logger << "possible.size(): " << possible.size() << endl;
        logger << "-----------end----------" << endl;
    }

    void bound(double to_cover)
    {
        auto prev_possible = possible;
        auto prev_offcuts = offcuts;
        remove_scenes_disjoint_with(possible, offcuts); // a prune
        possible.sort([](Scene *a, Scene *b) { // unit price from lower to higher
            return a->price / a->valid_area < b->price / b->valid_area;
        });
        price_lower_bound = price;
        double covered_upper_bound = covered;
        // take the lowest unit price, try to eliminate the roi offcuts, until all the possible run out or offcuts have been covered
        for (auto possible_scene : possible)
        {
            for (const auto &scene_offcut : possible_scene->offcuts)
            {
                for (auto it = offcuts.begin(); it != offcuts.end(); )
                {
                    const auto &offcut = *it;
                    auto inners = list<Polygon>();
                    auto outers = list<Polygon>();
                    tie(inners, outers) = clip(offcut, scene_offcut);
                    if (inners.size() > 0) // intersects
                    {
                        for (const auto &inner : inners)
                        {
                            double inner_area = area(inner);
                            covered_upper_bound += inner_area;
                            price_lower_bound += possible_scene->price / possible_scene->valid_area * inner_area;
                        }
                        for (const auto &outer : outers)
                        {
                            offcuts.push_back(outer);
                        }
                        it = offcuts.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }
            }
        }

        if (covered_upper_bound < to_cover) // the optimistic value is not able to covered the whole scene
        {
            price_lower_bound = numeric_limits<double>::max(); 
        }

        possible = prev_possible;
        offcuts = prev_offcuts;
    }
};

list<Scene *> optimize(ROI *roi, list<Scene *> possible_scenes, double target_coverage_ratio, double delta)
{
    Stopwatch sw;

    sw.restart();
    double roi_area = area(roi->poly);
    double to_cover = roi_area * target_coverage_ratio;
    auto optimal_state = shared_ptr<State>(new State{});
    {
        { // initialize optimal state
            auto selected = greedy::optimize(roi, possible_scenes, target_coverage_ratio, delta);
            optimal_state->price = accumulate(selected.begin(), selected.end(), 0.0, [](double acc, Scene *scene) {
                return acc + scene->price;
            });
            optimal_state->covered = to_cover;
            optimal_state->selected = move(selected);
        }
        { // initialize roi, possible scenes
            continuous::cut_roi(roi);
            continuous::remove_tiny_offcuts(roi->offcuts, delta);
            continuous::cut_scenes(possible_scenes, roi);
            for (auto possible_scene : possible_scenes)
            {
                continuous::remove_tiny_offcuts(possible_scene->offcuts, delta);
            }
            continuous::remove_scenes_with_no_offcuts(possible_scenes);
        }
    }
    g_report["t1"] = sw.lap();

    logger.debug(to_string(possible_scenes.size()));

    sw.restart();
    {
        auto initial_state = shared_ptr<State>(new State{});
        {
            initial_state->price = 0;
            initial_state->covered = 0;
            initial_state->possible = possible_scenes;
            initial_state->offcuts = roi->offcuts;
        }

        auto states = list<shared_ptr<State>>{initial_state};
        while (states.size() > 0)
        {
            auto state = states.front();
            states.pop_front();
            state->bound(to_cover);

            // logger << endl;
            // logger << states.size() << endl;
            // logger << "optimal price: " << optimal_state->price << endl;
            // state->print();

            if (state->covered > to_cover)
            {
                if (state->price < optimal_state->price) // lower price, great
                {
                    optimal_state = state;
                    logger << "better solution" << endl;
                    logger << "price: " << optimal_state->price << endl;
                    logger << "covered: " << optimal_state->covered / roi_area << endl;
                    logger << "scenes " << optimal_state->selected.size() << endl;
                }
                else
                {
                    // no lower price, keeping branching (add more scenes) makes no better, just kill it.
                }
            } 
            else if (state->price_lower_bound < optimal_state->price && state->possible.size() > 0)
            {
                states.splice(states.end(), state->branch(delta));
            }
        }
    }
    g_report["t2"] = sw.lap();
    return optimal_state->selected;
}
}
}
}
