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
        transform(roi, possible_scenes, delta);
    }
    g_report["t1"] = sw.lap();

    sw.restart();
    int covered = 0;
    int to_cover = roi->cell_set.size() * target_coverage_ratio;
    auto selected_scenes = list<Scene *>();
    {
        remove_scenes_with_no_cells(possible_scenes);
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
            remove_scenes_with_no_cells(possible_scenes);
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
        transform(roi, possible_scenes, delta);
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

namespace BFS
{
struct Node
{
    double price;             // the cost of selected scenes
    double uncovered;         // only if the covered is satisfied can the node be accepted as an optimal node
    double price_lower_bound; // if the lower bound higher than the optimal, kill it

    list<Scene *> selected;
    list<Scene *> possible;
    list<Polygon> offcuts;

    list<shared_ptr<Node>> branch(double delta)
    {
        list<shared_ptr<Node>> ret;

        { // discard
            auto node = shared_ptr<Node>(new Node(*this));
            node->possible.pop_front();
            ret.push_back(node);
        }

        { // adopt
            auto node = shared_ptr<Node>(new Node(*this));
            auto current_scene = possible.front();
            node->possible.pop_front();
            // offcuts & covered
            node->offcuts = difference(offcuts, current_scene->offcuts);
            // selected scenes
            node->selected.push_back(current_scene);
            // cost
            node->price += current_scene->price;
            // uncovered area
            node->uncovered = area(node->offcuts);
            remove_tiny_offcuts(node->offcuts, delta);
            // remove
            // remove_scenes_disjoint_with(node->possible, node->offcuts);
            ret.push_back(node);
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

    inline void bound_c(double acceptable_uncovered, double delta)
    {
        // since possible is sorted, there is no need to sort it again
        // possible.sort([](Scene *a, Scene *b) { // unit price from lower to higher
        //     return a->price / a->valid_area < b->price / b->valid_area;
        // });

        // initialize the bounds
        price_lower_bound = price;
        double to_cover = uncovered - acceptable_uncovered;
        // take the lowest unit price, try to eliminate the roi offcuts, until all the possible run out or offcuts have been covered
        // save the offcuts
        auto offcuts_backup = offcuts;
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
                                price_lower_bound += possible_scene->unit_price * to_cover;
                                to_cover = 0;
                            }
                            else
                            {
                                price_lower_bound += possible_scene->unit_price * inner_area;
                                to_cover -= inner_area;
                            }
                        }
                        for (const auto &outer : outers)
                        {
                            if (area(outer) >= delta * delta)
                            {
                                offcuts.push_back(outer);
                            }
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
        offcuts = offcuts_backup;
    }

    inline void bound(double acceptable_uncovered, double delta)
    {
        price_lower_bound = price;
    }

    inline void bound_b(double acceptable_uncovered, double delta)
    {
        double to_cover = uncovered - acceptable_uncovered;
        price_lower_bound = price;
        price_lower_bound += to_cover * possible.front()->unit_price;
    }

    void update_unit_price(const list<Polygon> &offcuts)
    {
        for (auto it = possible.begin(); it != possible.end();)
        {
            double inter_area = area(intersection((*it)->offcuts, offcuts));
            if (inter_area == 0)
            {
                it = possible.erase(it);
            }
            else
            {
                (*it)->unit_price = (*it)->price / inter_area;
                ++it;
            }
        }
    }

    // inline void bound(double acceptable_uncovered, double delta)
    // {
    //     update_unit_price(offcuts);
    //     possible.sort([](Scene *a, Scene *b) { // unit price from lower to higher
    //         return a->unit_price < b->unit_price;
    //     });

    //     // initialize the bounds
    //     price_lower_bound = price;
    //     double to_cover = uncovered - acceptable_uncovered;
    //     // take the lowest unit price, try to eliminate the roi offcuts, until all the possible run out or offcuts have been covered
    //     // save the offcuts
    //     auto offcuts_backup = offcuts;
    //     // default_random_engine gen;
    //     // uniform_real_distribution<double> dis(0, 1);
    //     double prev_unit_price = 0;
    //     double prev_slope = 0;
    //     for (auto possible_scene : possible)
    //     {
    //         if (to_cover <= 0)
    //             break;

    //         if (prev_unit_price > 0)
    //         {
    //             double slope = (possible_scene->price - prev_unit_price) / offcuts.size();
    //             if (slope <= prev_slope)
    //             {
    //                 price_lower_bound += to_cover * possible_scene->unit_price;
    //                 to_cover = 0;
    //                 break;
    //             }
    //             prev_slope = slope;
    //         }
    //         prev_unit_price = possible_scene->price;

    //         // // write something like this
    //         // {
    //         //     break;

    //         //     // loop
    //         //     // pick first one
    //         //     // cut first one against the rest
    //         //     // re-price and re-sort the rest

    //         //     // however, by doing so, this will become pure greedy ?
    //         //     // what's the different between it and pure greedy ?
    //         //     // or this question can be solved by greedy method ?
    //         //     // ????
    //         // } while (false);

    //         for (const auto &scene_offcut : possible_scene->offcuts) // modify this loop, the outer loop should be the sorted list
    //         {
    //             if (to_cover <= 0)
    //                 break;
    //             for (auto it = offcuts.begin(); it != offcuts.end();)
    //             {
    //                 const auto &offcut = *it;
    //                 auto inners = list<Polygon>();
    //                 auto outers = list<Polygon>();
    //                 tie(inners, outers) = clip(offcut, scene_offcut);
    //                 // after each choosing of ..., should we calculate the price again ? and re-sort the rest.
    //                 if (inners.size() > 0) // intersects (? is there any possibility that all offcuts of the scenes is inner and no need to do intersection at all)
    //                 {
    //                     for (const auto &inner : inners)
    //                     {
    //                         double inner_area = area(inner);
    //                         if (to_cover < inner_area)
    //                         {
    //                             price_lower_bound += possible_scene->unit_price * to_cover;
    //                             to_cover = 0;
    //                         }
    //                         else
    //                         {
    //                             price_lower_bound += possible_scene->unit_price * inner_area;
    //                             to_cover -= inner_area;
    //                         }
    //                     }
    //                     for (const auto &outer : outers)
    //                     {
    //                         if (area(outer) >= delta * delta)
    //                         {
    //                             offcuts.push_back(outer);
    //                         }
    //                     }
    //                     it = offcuts.erase(it);
    //                 }
    //                 else
    //                 {
    //                     ++it;
    //                 }
    //             }
    //         }
    //     }

    //     if (to_cover > 0) // the optimistic value is not able to covered the whole scene
    //     {
    //         price_lower_bound = numeric_limits<double>::max();
    //     }
    //     // restore the offcuts
    //     offcuts = offcuts_backup;
    // }
};

list<Scene *> optimize(ROI *roi, list<Scene *> possible_scenes, double target_coverage_ratio, double delta)
{
    Stopwatch sw;

    sw.restart();
    double roi_area = area(roi->poly);
    double acceptable_uncovered = roi_area * (1 - target_coverage_ratio);
    auto optimal_node = shared_ptr<Node>(new Node{});
    {
        { // initialize optimal node
            auto selected = greedy::optimize(roi, possible_scenes, target_coverage_ratio, delta);
            optimal_node->price = func::sum(selected, [](Scene *scene) { return scene->price; });
            optimal_node->uncovered = acceptable_uncovered;
            optimal_node->selected = move(selected);
            // optimal_node->price = numeric_limits<double>::max();
        }
        transform(roi, possible_scenes, delta);
    }
    g_report["t1"] = sw.lap();

    logger.debug(to_string(possible_scenes.size()));

    sw.restart();
    {
        auto initial_node = shared_ptr<Node>(new Node{});
        {
            initial_node->price = 0;
            initial_node->uncovered = roi_area;
            initial_node->offcuts = roi->offcuts;
            initial_node->possible = possible_scenes;
            for (auto scene : initial_node->possible)
            {
                scene->unit_price = scene->price / area(scene->offcuts);
            }
            initial_node->possible.sort([](Scene *a, Scene *b) { // unit price from lower to higher
                return a->unit_price < b->unit_price;
            });
        }

        initial_node->bound(acceptable_uncovered, delta);

        auto comp = [](const shared_ptr<Node> &a, const shared_ptr<Node> &b) {
            return a->price_lower_bound > b->price_lower_bound; // to make it a min heap
        };
        auto nodes = priority_queue<shared_ptr<Node>, vector<shared_ptr<Node>>, decltype(comp)>(comp);
        nodes.push(initial_node);

        auto debug_ts = list<double>();
        auto debug_nodes = list<double>();
        auto debug_plbs = list<double>();
        auto debug_pubs = list<double>();

        while (nodes.size() > 0)
        {
            auto node = nodes.top();
            nodes.pop();

            logger << nodes.size() << endl;
            debug_ts.push_back(sw.lap());
            debug_nodes.push_back(nodes.size());
            debug_plbs.push_back(node->price_lower_bound);
            debug_pubs.push_back(optimal_node->price);

            if (sw.lap() > 200)
                break;

            if (node->uncovered < acceptable_uncovered)
            {
                if (node->price < optimal_node->price) // lower price, great
                {
                    optimal_node = node;
                    logger << "better solution" << endl;
                    optimal_node->print(roi_area);
                }
                // else, no lower price, keeping branching (add more scenes) will be no better, just kill it.
            }
            else if (node->possible.size() > 0)
            {
                // Stopwatch test;
                auto new_nodes = node->branch(delta);
                // // logger << "test branch" << test.lap() << endl;
                for (auto new_node : new_nodes)
                {
                    // test.restart();
                    new_node->bound(acceptable_uncovered, delta);
                    // logger << "test bound" << test.lap() << endl;
                    // if (new_node->price_lower_bound < node->price_lower_bound - 1)
                    // {
                    //     node->print(roi_area);
                    //     new_node->print(roi_area);
                    //     throw "";
                    // }
                    if (new_node->price_lower_bound < optimal_node->price)
                    {
                        nodes.push(new_node);
                    }
                }
            }
        }

        debug_report.push_back({
            // {"price", node->price},
            // {"coverage_ratio", node->covered / roi_area},
            // {"selected.size()", node->selected.size()},
            // {"possible.size()", node->possible.size()},
            {"t", debug_ts},
            {"number of nodes", debug_nodes},
            {"price lower bound", debug_plbs},
            {"price upper bound", debug_pubs},
        });
    }
    g_report["t2"] = sw.lap();
    return optimal_node->selected;
}
}
}
}

namespace semantical
{
namespace greedy
{
list<Scene *> optimize(ROI *roi, list<Scene *> possible_scenes, double target_coverage_ratio, double delta)
{
    Stopwatch sw;

    sw.restart();
    auto area_table = transform(roi, possible_scenes, delta);
    auto area = [&area_table](const Scene* scene) {
        return ::area(scene->cell_set, area_table);
    };
    g_report["t1"] = sw.lap();

    sw.restart();
    double covered = 0;
    double to_cover = ::area(roi->poly) * target_coverage_ratio;
    auto selected_scenes = list<Scene *>();
    {
        remove_scenes_with_no_cells(possible_scenes);
        while (covered < to_cover && possible_scenes.size() > 0) // n
        {
            // select current
            // sw.restart();
            auto it = min_element(possible_scenes.begin(),
                                  possible_scenes.end(),
                                  [area](const Scene *a, const Scene *b) { // n
                                      return a->price / area(a) < b->price / area(b);
                                  });
            // g_report["t2.1"] = sw.lap();

            // claim the selected
            // sw.restart();
            auto selected_scene = *it;
            possible_scenes.erase(it);
            covered += area(selected_scene);
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
            remove_scenes_with_no_cells(possible_scenes);
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
