#include "optimizer.h"

#include <queue>

using namespace std;
using nlohmann::json;

Optimizer::Optimizer(double target_coverage) : target_coverage(target_coverage)
{
}

GreedyOptimizer::GreedyOptimizer(double target_coverage) : Optimizer(target_coverage)
{
}

json GreedyOptimizer::optimize(const Universe &universe, const Ranges &ranges, Ranges &result_ranges) const
{
    auto report = json();
    struct Pair
    {
        Range range;
        const Range *original_range;
    };
    result_ranges.clear();
    double current_value = 0;
    double target_value = universe.value * target_coverage;
    auto pairs = func::map(ranges, [](const auto &range) {
        return Pair{range, &range};
    });
    while (current_value < target_value && pairs.size() > 0) // n
    {
        auto it = func::min_element(pairs, [](const Pair &pair) { // n
            return pair.range.cost / pair.range.value;
        });
        auto current_range = move(it->range);
        result_ranges.push_back(*it->original_range);
        pairs.erase(it);

        current_value += current_range.value;
        for (auto &pair : pairs)
        {
            if (intersects(pair.range.entity->polygon, current_range.entity->polygon))
            {
                for (const auto &element : current_range.elements) // m
                {
                    pair.range.elements.erase(element); // 1
                }
                pair.range.update_value();
            }
        }
        pairs = func::filter(pairs, [](const Pair &pair) {
            return pair.range.elements.size() > 0;
        });
        // cout << current_value * 100.0 / const_universe.current_value << "%: " << pairs.size() << endl;
    }

    if (current_value < target_value)
    {
        result_ranges.clear();
    }

    return report;
}

struct BaseNode
{
    double cost = 0;  // the cost of selected ranges
    double value = 0; // current covered area
    int cursor = 0;   // next range to select (or not select)
    double cost_lower_bound = 0;
    vector<int> selected;

    static double target_value;
    static const Universe *universe;
    static vector<Range> ranges;

    vector<shared_ptr<BaseNode>> branch()
    {
        vector<shared_ptr<BaseNode>> ret;

        { // not select
            auto node = this->clone();
            node->drop_next();
            ret.push_back(node);
        }

        { // select
            auto node = this->clone();
            node->select_next();
            ret.push_back(node);
        }
        return ret;
    }

    void drop_next()
    {
        ++cursor;
    }

    void select_next()
    {
        selected.push_back(cursor);
        cost += ranges[cursor].cost;
        update_value();
        ++cursor;
    }

    virtual shared_ptr<BaseNode> clone() = 0;
    virtual void update_value() = 0;
    virtual void bound() = 0;

    virtual nlohmann::json describe()
    {
        assert(universe != nullptr);
        return nlohmann::json{
            {"cost", cost},
            {"to_select.size()", ranges.size() - cursor},
            {"selected.size()", selected.size()},
            {"coverage_ratio", value / universe->value},
            {"cost lower bound", cost_lower_bound},
        };
    }

    void print(const string &s = "")
    {
        cout << s << endl;
        auto desc = describe();
        cout << desc.dump(4) << endl;
    }
};

double BaseNode::target_value = 0;
const Universe *BaseNode::universe = nullptr;
vector<Range> BaseNode::ranges;

template <class Node>
json branch_and_bound(const Universe &universe, const Ranges &ranges, Ranges &result_ranges, double target_coverage)
{
    auto report = json();
    auto sw = Stopwatch(); // timer
    Node::ranges = vector<Range>(ranges.begin(), ranges.end());
    sort(Node::ranges.begin(), Node::ranges.end(), [](const Range &a, const Range &b) { // unit cost from lower to higher
        return a.cost / a.value < b.cost / b.value;
    });
    Node::target_value = universe.value * target_coverage;
    Node::universe = &universe;

    auto optimal_node = shared_ptr<Node>(new Node{});
    {
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
        cout << "number of nodes: " << nodes.size() << endl;
        auto desc = node->describe();
        desc["t"] = t; // to avoid lag
        desc["number of nodes"] = nodes.size();
        desc["cost upper bound"] = optimal_node->cost;
        report.push_back(desc);
        sw.continue_();

        if (node->value >= Node::target_value)
        {
            if (node->cost < optimal_node->cost) // lower cost, great
            {
                optimal_node = node;
                optimal_node->print("better solution");
            }
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
    return report;
}

BnbOptimizer::BnbOptimizer(double target_coverage) : Optimizer(target_coverage)
{
}

json BnbOptimizer::optimize(const Universe &universe, const Ranges &ranges, Ranges &result_ranges) const
{
    struct Node : public BaseNode
    {
        vector<bool> visited;

        shared_ptr<BaseNode> clone()
        {
            return make_shared<Node>();
        }

        void update_value()
        {
            for (const auto &element : ranges[cursor].elements)
            {
                if (visited[element.index] == 0)
                {
                    visited[element.index] = 1;
                    value += element.value;
                }
            }
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

        json describe()
        {
            int n_visited = 0;
            for (int i = 0; i < visited.size(); ++i)
            {
                n_visited += visited[i];
            }
            auto ret = BaseNode::describe();
            ret["visited.size()"] = n_visited;
            return ret;
        }
    };
}

OnlineBnbOptimizer::OnlineBnbOptimizer(double target_coverage) : Optimizer(target_coverage)
{
}

json OnlineBnbOptimizer::optimize(const Universe &universe, const Ranges &ranges, Ranges &result_ranges) const
{
    struct Node : BaseNode
    {
        list<Polygon> offcuts;

        shared_ptr<BaseNode> clone()
        {
            return make_shared<Node>();
        }

        void update_value()
        {
            assert(cursor < ranges.size() && "cursor out of ranges!");
            // offcuts & covered
            offcuts = difference(offcuts, {ranges[cursor].entity->polygon});
            // deficit area
            value = universe->value - func::sum(offcuts, area);
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
                    auto scene_offcuts = intersection(universe->entity->polygon, range.entity->polygon);
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

        json describe()
        {
            auto ret = BaseNode::describe();
            ret["visited.size()"] = offcuts.size();
            return ret;
        }
    };
}