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
    auto universe = const_universe;
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
    double cost = 0;  // the cost of selected ranges
    double value = 0; // current covered area
    double cost_lower_bound;

    list<int> selected;
    int cursor = 0;
    vector<bool> visited;

    static double target_value;
    static const Range *universe;
    static vector<Range> ranges;

    list<shared_ptr<Node>> branch()
    {
        list<shared_ptr<Node>> ret;

        { // not select
            auto node = shared_ptr<Node>(new Node(*this));
            ++node->cursor;
            ret.push_back(node);
        }

        { // select
            auto node = shared_ptr<Node>(new Node(*this));
            // offcuts & value
            const auto &range = ranges[cursor];
            for (const auto &element : range.elements)
            {
                if (node->visited[element.index] == 0)
                {
                    node->visited[element.index] = 1;
                    node->value += element.value;
                }
            }
            // selected ranges
            node->selected.push_back(cursor);
            // cost
            node->cost += range.cost;
            ++node->cursor;
            ret.push_back(node);
        }
        return ret;
    }

    void bound()
    {
        cost_lower_bound = cost;
        double current_value = value;
        auto visited_copy = visited;
        for (int i = cursor; i < ranges.size(); ++i)
        {
            const auto &range = ranges[i];
            for (const auto &element : range.elements) // k
            {
                if (visited_copy[element.index] == 0)
                {
                    double deficit = target_value - current_value;
                    if (element.value > deficit)
                    {
                        cost_lower_bound += range.cost / range.value * deficit;
                        current_value += deficit;
                        return;
                    }
                    else
                    {
                        cost_lower_bound += range.cost / range.value * element.value;
                        current_value += element.value;
                    }
                    visited_copy[element.index] = 1;
                }
            }
        }
        if (current_value < target_value)
        {
            cout << current_value << " " << target_value << endl;
            cost_lower_bound = numeric_limits<double>::max();
        }
    }

    nlohmann::json describe()
    {
        int n_visited = 0;
        for (int i = 0; i < visited.size(); ++i)
        {
            n_visited += visited[i];
        }
        return nlohmann::json{
            {"cost", cost},
            {"to_select.size()", ranges.size() - cursor},
            {"selected.size()", selected.size()},
            {"visited.size()", n_visited},
            {"coverage_ratio", value / universe->value},
            {"cost lower bound", cost_lower_bound},
        };
    }

    void print(const string &s = "")
    {
        assert(universe != nullptr);
        logger << s << endl;
        auto desc = describe();
        for (auto it = desc.begin(); it != desc.end(); ++it)
        {
            logger << it.key() << ": " << it.value() << endl;
        }
    }
};

double Node::target_value = 0;
const Range *Node::universe = nullptr;
vector<Range> Node::ranges;

Bnb::Bnb(const Range &const_universe, const list<Range> &const_ranges, double target_coverage)
{
    auto sw = Stopwatch(); // timer
    auto universe = const_universe;
    Node::ranges = vector<Range>(const_ranges.begin(), const_ranges.end());
    sort(Node::ranges.begin(), Node::ranges.end(), [](const Range &a, const Range &b) { // unit cost from lower to higher
        return a.cost / a.value < b.cost / b.value;
    });
    Node::target_value = universe.value * target_coverage;
    Node::universe = &universe;
    auto optimal_node = shared_ptr<Node>(new Node{});
    {
        auto greedy = Greedy(const_universe, const_ranges, target_coverage);
        optimal_node->cost = func::sum(greedy.subranges,
                                       [](list<Range>::const_iterator rit) {
                                           return rit->cost;
                                       });
        optimal_node->print("initial optimal");
    }
    auto initial_node = shared_ptr<Node>(new Node{});
    {
        initial_node->cost = 0;
        initial_node->value = 0;
        initial_node->cursor = 0;
        initial_node->visited = vector<bool>(universe.elements.size());
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

        sw.pause(); // pause the timer for debug informaiton
        double t = sw.lap();
        logger << nodes.size() << endl;
        auto desc = node->describe();
        desc["t"] = t; // to avoid lag
        desc["number of nodes"] = nodes.size();
        desc["cost upper bound"] = optimal_node->cost;
        debug_report.push_back(desc);
        sw.continue_();

        if (node->value >= Node::target_value) // finished cover
        {
            if (node->cost < optimal_node->cost) // lower cost, great
            {
                optimal_node = node;
                optimal_node->print("better solution");
            }
            // else keeping branching (add more ranges) will be no good, just kill the node.
        }
        else if (node->cursor != Node::ranges.size())
        {
            auto new_nodes = node->branch();
            for (auto new_node : new_nodes)
            {
                new_node->bound();
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

struct Node
{
    double cost = 0;  // the cost of selected ranges
    double value = 0; // current covered area
    double cost_lower_bound;

    list<int> selected;
    int cursor = 0; // next range to select (or not select)

    static double target_value;
    static const Range *universe;
    static vector<Range> ranges;

    list<Polygon> offcuts;

    list<shared_ptr<Node>> branch()
    {
        list<shared_ptr<Node>> ret;

        { // not select
            auto node = shared_ptr<Node>(new Node(*this));
            ++node->cursor;
            ret.push_back(node);
        }

        { // select
            auto node = shared_ptr<Node>(new Node(*this));
            const auto &range = ranges[cursor];
            // offcuts & covered
            node->offcuts = difference(offcuts, {range.entity->poly});
            // selected ranges
            node->selected.push_back(cursor);
            // cost
            node->cost += range.cost;
            // deficit area
            node->value = universe->value - func::sum(node->offcuts, area);
            // remove
            ++node->cursor;
            ret.push_back(node);
        }
        return ret;
    }

    void bound()
    {
        cost_lower_bound = cost;
        auto left_offcuts = offcuts;
        double current_value = value;
        for (int i = cursor; i < ranges.size(); ++i)
        {
            for (auto it = left_offcuts.begin(); it != left_offcuts.end();)
            {
                const auto &range = ranges[i];
                auto scene_offcuts = intersection(universe->entity->poly, range.entity->poly);
                if (scene_offcuts.size() == 0) // no intersection
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
                            cost_lower_bound += range.cost / range.value * deficit;
                            current_value += deficit;
                            return; // value is enough, return
                        }
                        else
                        {
                            cost_lower_bound += range.cost / range.value * inner_area;
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

    nlohmann::json describe()
    {
        return nlohmann::json{
            {"cost", cost},
            {"to_select.size()", ranges.size() - cursor},
            {"selected.size()", selected.size()},
            {"offcuts.size()", offcuts.size()},
            {"coverage_ratio", value / universe->value},
            {"cost lower bound", cost_lower_bound},
        };
    }

    void print(const string &s = "")
    {
        assert(universe != nullptr);
        logger << s << endl;
        auto desc = describe();
        for (auto it = desc.begin(); it != desc.end(); ++it)
        {
            logger << it.key() << ": " << it.value() << endl;
        }
    }
};

double Node::target_value = 0;
const Range *Node::universe = nullptr;
vector<Range> Node::ranges;

Bnb::Bnb(const Range &const_universe, const list<Range> &const_ranges, double target_coverage)
{
    auto sw = Stopwatch(); // timer
    auto universe = const_universe;
    Node::ranges = vector<Range>(const_ranges.begin(), const_ranges.end());
    sort(Node::ranges.begin(), Node::ranges.end(), [](const Range &a, const Range &b) { // unit cost from lower to higher
        return a.cost / a.value < b.cost / b.value;
    });
    Node::target_value = area(universe.entity->poly) * target_coverage;
    Node::universe = &universe;

    auto optimal_node = shared_ptr<Node>(new Node{});
    {
        // optimal_node->cost = 24000.0;
        optimal_node->cost = 10000;
        optimal_node->print("initial optimal");
    }

    auto initial_node = shared_ptr<Node>(new Node{});
    {
        initial_node->cost = 0;
        initial_node->value = 0;
        initial_node->cursor = 0;
        initial_node->bound();
        initial_node->print("initial node");
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

        sw.pause(); // pause the timer for debug informaiton
        double t = sw.lap();
        logger << nodes.size() << endl;
        auto desc = node->describe();
        desc["t"] = t; // to avoid lag
        desc["number of nodes"] = nodes.size();
        desc["cost upper bound"] = optimal_node->cost;
        debug_report.push_back(desc);
        sw.continue_();

        if (node->value >= Node::target_value)
        {
            if (node->cost < optimal_node->cost) // lower cost, great
            {
                optimal_node = node;
                optimal_node->print("better solution");
            }
            // else, no lower cost, keeping branching (add more ranges) will be no better, just kill it.
        }
        else if (node->cursor != Node::ranges.size())
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