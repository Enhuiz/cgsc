#include "solution.h"

#include <algorithm>
#include <list>
#include <iostream>
#include <functional>
#include <queue>

#include "global.h"

using namespace std;

double calculate_density(const Entity &roi, const list<Entity> &entities)
{
    return func::sum(entities, [roi](const Entity &entity) {
               return func::sum(intersection(entity.poly, roi.poly),
                                [](const Polygon &poly) {
                                    return area(poly);
                                });
           }) /
           area(roi.poly);
}

Greedy::Greedy(const Range &const_universe, const list<Range> &const_ranges, double target_coverage)
{
    auto universe = Range(const_universe);
    auto ranges = list<Range>(const_ranges.begin(), const_ranges.end());
    double value = 0;
    double target_value = universe.value * target_coverage;

    subranges.clear();
    while (value < target_value && ranges.size() > 0) // n
    {
        auto cur_it = func::min_element(ranges, [](const Range &range) { // n
            return range.cost / range.value;
        });

        // claim the selected
        // sw.restart();
        auto current_range = *cur_it;
        ranges.erase(cur_it);
        subranges.push_back(cur_it);
        value += current_range.value;
        // g_report["t2.2"] = sw.lap();

        // update rest
        // remove cells from the left to_select ranges
        // sw.restart();
        for (auto it = ranges.begin(); it != ranges.end();)
        {
            auto &range = *it;
            if (intersects(range.entity->poly, current_range.entity->poly))
            {
                for (const auto &element : current_range.elements) // m
                {
                    range.elements.erase(element); // 1
                }

                if (range.elements.size() == 0)
                {
                    it = ranges.erase(it);
                }
                else
                {
                    range.update_value();
                    ++it;
                }
            }
            else
            {
                ++it;
            }
        }
        cerr << value * 100.0 / universe.value << "%: " << ranges.size() << endl;
    }

    if (value < target_value)
    {
        subranges.clear();
    }
}

struct Node
{
    double cost = 0;         // the cost of selected ranges
    double value = 0;        // only if the covered is satisfied can the node be accepted as an optimal node
    double cost_lower_bound; // if the lower bound higher than the optimal, kill it

    using RangeConstIter = list<Range>::const_iterator;

    list<RangeConstIter> selected;
    list<RangeConstIter> to_select;
    unordered_set<Element> elements;

    static double target_value;
    static const Range *universe;

    list<shared_ptr<Node>> branch()
    {
        list<shared_ptr<Node>> ret;

        { // discard
            auto node = shared_ptr<Node>(new Node(*this));
            node->to_select.pop_front();
            ret.push_back(node);
        }

        { // adopt
            auto node = shared_ptr<Node>(new Node(*this));
            auto current = to_select.front();
            node->to_select.pop_front();
            // offcuts & covered
            for (const auto &element : current->elements)
            {
                node->elements.insert(element);
            }
            // selected ranges
            node->selected.push_back(current);
            // cost
            node->cost += current->cost;
            // deficit area
            node->value = func::sum(node->elements, [](const Element &e) {
                return e.value;
            });
            // remove
            ret.push_back(node);
        }
        return ret;
    }

    inline void bound()
    {
        cost_lower_bound = cost;
        double current_value = value;
        auto visited_elements = elements;
        for (const auto &rit : to_select) // n
        {
            for (const auto &element : rit->elements) // k
            {
                if (visited_elements.count(element) == 0)
                {
                    double deficit = target_value - current_value;
                    if (element.value > deficit)
                    {
                        cost_lower_bound += rit->cost / rit->value * deficit;
                        current_value += deficit;
                        return;
                    }
                    else
                    {
                        cost_lower_bound += rit->cost / rit->value * element.value;
                        current_value += element.value;
                    }
                    visited_elements.insert(element);
                }
            }
        }
        if (current_value < target_value)
        {
            cout << current_value << " " << target_value << endl;
            cost_lower_bound = numeric_limits<double>::max();
        }
    }

    void print(const string &s = "")
    {
        assert(universe != nullptr);
        logger << s << endl;
        logger << "cost: " << cost << endl;
        logger << "coverage ratio: " << value / universe->value << endl;
        logger << "cost_lower_bound: " << cost_lower_bound << endl;
        logger << "elements.size(): " << elements.size() << endl;
        logger << "to_select.size(): " << to_select.size() << endl;
        logger << "selected.size(): " << selected.size() << endl;
    }
};

double Node::target_value = 0;
const Range *Node::universe = nullptr;

Bnb::Bnb(const Range &const_universe, const list<Range> &const_ranges, double target_coverage)
{
    auto sw = Stopwatch();
    auto universe = Range(const_universe);
    auto ranges = list<Range>(const_ranges.begin(), const_ranges.end());
    Node::target_value = universe.value * target_coverage;
    Node::universe = &universe;
    auto optimal_node = shared_ptr<Node>(new Node{});
    {
        auto greedy = Greedy(const_universe, const_ranges, target_coverage);
        // optimal_node->cost = func::sum(greedy.subranges,
        //                                [](decltype(ranges)::const_iterator rit) {
        //                                    return rit->cost;
        //                                });
        optimal_node->cost = 10000;
        optimal_node->print("initial optimal");
    }
    auto initial_node = shared_ptr<Node>(new Node{});
    {
        initial_node->cost = 0;
        initial_node->value = 0;
        for (auto it = ranges.begin(); it != ranges.end(); ++it)
        {
            initial_node->to_select.push_back(it);
        }
        initial_node->to_select.sort([](list<Range>::const_iterator a,
                                        list<Range>::const_iterator b) {
            return a->cost / a->value < b->cost / b->value;
        });
        initial_node->bound();
        initial_node->print("initial node");
    }
    auto comp = [](const shared_ptr<Node> &a, const shared_ptr<Node> &b) {
        return a->cost_lower_bound > b->cost_lower_bound; // to make it a min heap
    };
    auto nodes = priority_queue<shared_ptr<Node>, vector<shared_ptr<Node>>, decltype(comp)>(comp);
    nodes.push(initial_node);
    while (nodes.size() > 0 && sw.lap() < 200)
    {
        auto node = nodes.top();
        nodes.pop();

        logger << nodes.size() << endl;

        debug_report.push_back({
            {"cost", node->cost},
            {"coverage_ratio", node->value / universe.value},
            {"selected.size()", node->selected.size()},
            {"to_select.size()", node->to_select.size()},
            {"t", sw.lap()},
            {"number of nodes", nodes.size()},
            {"cost lower bound", node->cost_lower_bound},
            {"cost upper bound", optimal_node->cost},
        });

        if (node->value >= Node::target_value) // finished cover
        {
            if (node->cost < optimal_node->cost) // lower cost, great
            {
                optimal_node = node;
                logger << "better solution" << endl;
                optimal_node->print();
            }
            // else keeping branching (add more ranges) will be no better, just kill it.
        }
        else if (node->to_select.size() > 0)
        {
            // auto new_nodes = timeit("branch", [&node]() { return node->branch(); });
            auto new_nodes = node->branch();
            for (auto new_node : new_nodes)
            {
                // timeit("bound", [&new_node]() { new_node->bound(); return 0;});
                new_node->bound();
                // if (new_node->cost_lower_bound < node->cost_lower_bound - 1)
                // {
                //     node->print(roi_area);
                //     new_node->print(roi_area);
                //     throw "";
                // }
                if (new_node->cost_lower_bound < optimal_node->cost)
                {
                    nodes.push(new_node);
                }
            }
        }
    }
}

namespace old
{
void remove_ranges_disjoint_with(list<list<Range>::const_iterator> &ranges, const list<Polygon> &polys)
{
    ranges.erase(remove_if(ranges.begin(),
                           ranges.end(),
                           [&polys](list<Range>::const_iterator rit) {
                               return none_of(polys.begin(), polys.end(), [rit](const Polygon &poly) {
                                   return intersects(rit->entity->poly, poly);
                               });
                           }),
                 ranges.end());
}

struct Node
{
    double cost;             // the cost of selected ranges
    double value;          
    double cost_lower_bound; // if the lower bound higher than the optimal, kill it

    using RangeConstIter = list<Range>::const_iterator;

    list<RangeConstIter> selected;
    list<RangeConstIter> to_select;
    list<Polygon> offcuts;

    static double target_value;
    static const Range *universe;

    list<shared_ptr<Node>> branch()
    {
        list<shared_ptr<Node>> ret;

        { // discard
            auto node = shared_ptr<Node>(new Node(*this));
            node->to_select.pop_front();
            ret.push_back(node);
        }

        { // adopt
            auto node = shared_ptr<Node>(new Node(*this));
            auto current_range = to_select.front();
            node->to_select.pop_front();
            // offcuts & covered
            node->offcuts = difference(offcuts, {current_range->entity->poly});
            // selected ranges
            node->selected.push_back(current_range);
            // cost
            node->cost += current_range->cost;
            // deficit area
            node->value = universe->value - func::sum(node->offcuts, area);
            // remove
            // remove_ranges_disjoint_with(node->to_select, node->offcuts);
            ret.push_back(node);
        }
        return ret;
    }

    void print()
    {
        assert(universe != nullptr);
        logger << "cost: " << cost << endl;
        logger << "coverage ratio: " << value / universe->value << endl;
        logger << "cost_lower_bound: " << cost_lower_bound << endl;
        logger << "offcuts.size(): " << offcuts.size() << endl;
        logger << "to_select.size(): " << to_select.size() << endl;
        logger << "selected.size(): " << selected.size() << endl;
    }

    void bound()
    {
        cost_lower_bound = cost;
        auto left_offcuts = offcuts;
        double current_value = value;
        for (auto rit : to_select)
        {
            for (auto it = left_offcuts.begin(); it != left_offcuts.end();)
            {
                auto scene_offcuts = intersection(universe->entity->poly, rit->entity->poly);
                if (scene_offcuts.size() == 0)
                    continue;
                auto scene_offcut = scene_offcuts.front();
                const auto &left_offcut = *it;
                auto inners = list<Polygon>();
                auto outers = list<Polygon>();
                tie(inners, outers) = clip(left_offcut, scene_offcut);
                if (inners.size() > 0) // intersects
                {
                    for (const auto &inner : inners)
                    {
                        double deficit = target_value - current_value;
                        double inner_area = area(inner);
                        if (deficit <= inner_area)
                        {
                            cost_lower_bound += rit->cost / rit->value * deficit;
                            current_value += deficit;
                            return;
                        }
                        else
                        {
                            cost_lower_bound += rit->cost / rit->value * inner_area;
                            current_value += inner_area;
                        }
                    }
                    for (const auto &outer : outers)
                    {
                        left_offcuts.push_back(outer);
                    }
                    it = left_offcuts.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
        if (current_value < target_value) // the optimistic value is not able to covered the whole scene
        {
            cost_lower_bound = numeric_limits<double>::max();
        }
    }
};

double Node::target_value = 0;
const Range *Node::universe = nullptr;

Bnb::Bnb(const Range &const_universe, const list<Range> &const_ranges, double target_coverage)
{
    auto sw = Stopwatch();
    auto universe = const_universe;
    auto ranges = const_ranges;
    Node::target_value = area(universe.entity->poly) * target_coverage;
    Node::universe = &universe;

    auto optimal_node = shared_ptr<Node>(new Node{});
    {
        optimal_node->cost = 10000;
    }

    auto initial_node = shared_ptr<Node>(new Node{});
    {
        initial_node->cost = 0;
        initial_node->value = 0;
        initial_node->offcuts = {universe.entity->poly};
        for (auto it = ranges.begin(); it != ranges.end(); ++it)
        {
            initial_node->to_select.push_back(it);
        }
        initial_node->to_select.sort([](list<Range>::const_iterator a, list<Range>::const_iterator b) { // unit cost from lower to higher
            return a->cost / a->value < b->cost / b->value;
        });
        initial_node->bound();
    }

    auto comp = [](const shared_ptr<Node> &a, const shared_ptr<Node> &b) {
        return a->cost_lower_bound > b->cost_lower_bound; // to make it a min heap
    };

    auto nodes = priority_queue<shared_ptr<Node>, vector<shared_ptr<Node>>, decltype(comp)>(comp);
    nodes.push(initial_node);

    while (nodes.size() > 0 && sw.lap() <= 200)
    {
        auto node = nodes.top();
        nodes.pop();

        logger << nodes.size() << endl;
        sw.pause();
        debug_report.push_back({
            {"cost", node->cost},
            {"coverage_ratio", node->value / universe.value},
            {"selected.size()", node->selected.size()},
            {"to_select.size()", node->to_select.size()},
            {"t", sw.lap()},
            {"number of nodes", nodes.size()},
            {"cost lower bound", node->cost_lower_bound},
            {"cost upper bound", optimal_node->cost},
        });
        sw.continue_();

        if (node->value >= Node::target_value)
        {
            if (node->cost < optimal_node->cost) // lower cost, great
            {
                optimal_node = node;
                logger << "better solution" << endl;
                optimal_node->print();
            }
            // else, no lower cost, keeping branching (add more ranges) will be no better, just kill it.
        }
        else if (node->to_select.size() > 0)
        {
            auto new_nodes = node->branch();
            for (auto new_node : new_nodes)
            {
                new_node->bound();
                // if (new_node->cost_lower_bound < node->cost_lower_bound - 1)
                // {
                //     node->print(roi_area);
                //     new_node->print(roi_area);
                //     throw "";
                // }
                if (new_node->cost_lower_bound < optimal_node->cost)
                {
                    nodes.push(new_node);
                }
            }
        }
    }
}
}

// inline void bound(double acceptable_uncovered, double delta)
// {
//     update_unit_cost(offcuts);
//     to_select.sort([](Entity *a, Entity *b) { // unit cost from lower to higher
//         return a->unit_cost < b->unit_cost;
//     });

//     // initialize the bounds
//     cost_lower_bound = cost;
//     double to_cover = deficit - acceptable_uncovered;
//     // take the lowest unit cost, try to eliminate the roi offcuts, until all the to_select run out or offcuts have been covered
//     // save the offcuts
//     auto offcuts_backup = offcuts;
//     // default_random_engine gen;
//     // uniform_real_distribution<double> dis(0, 1);
//     double prev_unit_cost = 0;
//     double prev_slope = 0;
//     for (auto possible_scene : to_select)
//     {
//         if (to_cover <= 0)
//             break;

//         if (prev_unit_cost > 0)
//         {
//             double slope = (possible_scene->cost - prev_unit_cost) / offcuts.size();
//             if (slope <= prev_slope)
//             {
//                 cost_lower_bound += to_cover * possible_scene->unit_cost;
//                 to_cover = 0;
//                 break;
//             }
//             prev_slope = slope;
//         }
//         prev_unit_cost = possible_scene->cost;

//         // // write something like this
//         // {
//         //     break;

//         //     // loop
//         //     // pick first one
//         //     // cut first one against the rest
//         //     // re-cost and re-sort the rest

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
//                 // after each choosing of ..., should we calculate the cost again ? and re-sort the rest.
//                 if (inners.size() > 0) // intersects (? is there any possibility that all offcuts of the ranges is inner and no need to do intersection at all)
//                 {
//                     for (const auto &inner : inners)
//                     {
//                         double inner_area = area(inner);
//                         if (to_cover < inner_area)
//                         {
//                             cost_lower_bound += possible_scene->unit_cost * to_cover;
//                             to_cover = 0;
//                         }
//                         else
//                         {
//                             cost_lower_bound += possible_scene->unit_cost * inner_area;
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
//         cost_lower_bound = numeric_limits<double>::max();
//     }
//     // restore the offcuts
//     offcuts = offcuts_backup;
// }