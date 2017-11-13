#include "solution.h"

#include <algorithm>
#include <list>
#include <iostream>
#include <functional>
#include <queue>

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
    double price;             // the price of selected scenes
    double uncovered;         // only if the covered is satisfied can the state be accepted as an optimal state
    double price_lower_bound; // if the lower bound higher than the optimal, kill it

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
            // offcuts & covered
            state->offcuts = difference(offcuts, current_scene->offcuts);
            double prev_uncovered = state->uncovered;
            state->uncovered = continuous::area(state->offcuts);
            if (state->uncovered < prev_uncovered)
            {
                continuous::remove_tiny_offcuts(state->offcuts, delta);
                // price
                state->price += current_scene->price;
                // selected scenes
                state->selected.push_back(current_scene);
                // ret
                ret.push_back(state);
            }
        }

        return ret;
    }

    void print(double roi_area)
    {
        logger << "price: " << price << endl;
        logger << "covered: " << 1 - uncovered / roi_area << endl;
        logger << "price_lower_bound: " << price_lower_bound << endl;
        logger << "offcuts.size(): " << offcuts.size() << endl;
        logger << "possible.size(): " << possible.size() << endl;
        logger << "selected.size(): " << selected.size() << endl;
    }

    void bound_(double acceptable_uncovered)
    {
        possible.sort([](Scene *a, Scene *b) { // unit price from lower to higher
            return a->price / a->valid_area < b->price / b->valid_area;
        });
        // initialize the bounds
        price_lower_bound = price;
        double to_cover = uncovered - acceptable_uncovered;
        // take the lowest unit price, try to eliminate the roi offcuts, until all the possible run out or offcuts have been covered
        // save the offcuts
        auto prev_offcuts = offcuts;
        for (auto possible_scene : possible)
        {
            if (to_cover <= 0)
                break;
            for (const auto &scene_offcut : possible_scene->offcuts)
            {
                if (to_cover <= 0)
                    break;
                for (auto it = offcuts.begin(); it != offcuts.end();)
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
                            if (to_cover < inner_area)
                            {
                                price_lower_bound += possible_scene->price / possible_scene->valid_area * to_cover;
                                to_cover = 0;
                            }
                            else
                            {
                                price_lower_bound += possible_scene->price / possible_scene->valid_area * inner_area;
                                to_cover -= inner_area;
                            }
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
        if (to_cover > 0) // the optimistic value is not able to covered the whole scene
        {
            price_lower_bound = numeric_limits<double>::max();
        }
        // restore the offcuts
        offcuts = prev_offcuts;
    }

    void bound(double acceptable_uncovered)
    {
        price_lower_bound = price;
    }
};

list<Scene *> optimize(ROI *roi, list<Scene *> possible_scenes, double target_coverage_ratio, double delta)
{
    Stopwatch sw;

    sw.restart();
    double roi_area = area(roi->poly);
    double acceptable_uncovered = roi_area * (1 - target_coverage_ratio);
    auto optimal_state = shared_ptr<State>(new State{});
    {
        { // initialize optimal state
            auto selected = greedy::optimize(roi, possible_scenes, target_coverage_ratio, delta);
            optimal_state->price = accumulate(selected.begin(),
                                              selected.end(),
                                              0.0,
                                              [](double acc, Scene *scene) {
                                                  return acc + scene->price;
                                              });
            optimal_state->uncovered = acceptable_uncovered;
            optimal_state->selected = move(selected);
            // optimal_state->price = numeric_limits<double>::max();
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
            initial_state->uncovered = roi_area;
            initial_state->possible = possible_scenes;
            initial_state->offcuts = roi->offcuts;
        }
        initial_state->bound(acceptable_uncovered);

        // auto states = list<shared_ptr<State>>{initial_state};
        auto comp = [](const shared_ptr<State> &a, const shared_ptr<State> &b) {
            return a->price_lower_bound > b->price_lower_bound; // to make it a min heap
        };
        auto states = priority_queue<shared_ptr<State>, vector<shared_ptr<State>>, decltype(comp)>(comp);
        states.push(initial_state);

        auto debug_ts = list<double>();
        auto debug_nodes = list<double>();
        auto debug_plbs = list<double>();
        auto debug_pubs = list<double>();

        while (states.size() > 0)
        {
            auto state = states.top();
            states.pop();

            logger << states.size() << endl;
            debug_ts.push_back(sw.lap());
            debug_nodes.push_back(states.size());
            debug_plbs.push_back(state->price_lower_bound);
            debug_pubs.push_back(optimal_state->price);

            if (sw.lap() > 200)
                break;

            if (state->uncovered < acceptable_uncovered)
            {
                if (state->price < optimal_state->price) // lower price, great
                {
                    optimal_state = state;
                    logger << "better solution" << endl;
                    optimal_state->print(roi_area);
                }
                // else, no lower price, keeping branching (add more scenes) will be no better, just kill it.
            }
            else if (state->possible.size() > 0)
            {
                auto new_states = state->branch(delta);
                for (auto new_state : new_states)
                {
                    new_state->bound(acceptable_uncovered);
                    // if (new_state->price_lower_bound < state->price_lower_bound - 1)
                    // {
                    //     state->print(roi_area);
                    //     new_state->print(roi_area);
                    //     throw "";
                    // }
                    if (new_state->price_lower_bound < optimal_state->price)
                    {
                        states.push(new_state);
                    }
                }
            }
        }
        debug_report.push_back({
            // {"price", state->price},
            // {"coverage_ratio", state->covered / roi_area},
            // {"price_lower_bound", state->price_lower_bound},
            // {"selected.size()", state->selected.size()},
            // {"possible.size()", state->possible.size()},
            // {"number of nodes", states.size()},
            // {"price_upper_bound", optimal_state->price},
            // {"t", sw.lap()},
            {"t", debug_ts},
            {"number of nodes", debug_nodes},
            {"price_lower_bound", debug_plbs},
            {"price_upper_bound", debug_pubs},
        });
    }
    g_report["t2"] = sw.lap();
    return optimal_state->selected;
}
}
}
}
