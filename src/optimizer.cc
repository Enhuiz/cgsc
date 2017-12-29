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
    report.clear();
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
            if (intersects(pair.range.product->polygon, current_range.product->polygon))
            {
                for (const auto &element : current_range.elements) // m
                {
                    pair.range.elements.erase(element); // 1
                }
                pair.range.update_value();
            }
        }
        pairs = func::filter(pairs, [](const Pair &pair) {
            return pair.range.value > 0;
        });
        // cout << current_value * 100.0 / universe.value << "%: " << pairs.size() << endl;
    }
    if (current_value < target_value)
    {
        result_ranges.clear();
    }
    report["actual_coverage"] = current_value / universe.value;
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

    virtual shared_ptr<BaseNode> clone() = 0;
    virtual void update_value() = 0;

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
void branch_and_bound(const Universe &universe, const Ranges &ranges, Ranges &result_ranges, double target_coverage, json &report)
{
    auto sw = Stopwatch(); // timer
    Node::ranges = vector<Range>(ranges.begin(), ranges.end());
    sort(Node::ranges.begin(), Node::ranges.end(), [](const Range &a, const Range &b) { // unit cost from lower to higher
        return a.cost / a.value < b.cost / b.value;
    });
    Node::target_value = universe.value * target_coverage;
    Node::universe = &universe;

    shared_ptr<BaseNode> optimal_node = make_shared<Node>();
    {
        auto greedy_optimizer = GreedyOptimizer(target_coverage);
        greedy_optimizer.optimize(universe, ranges, result_ranges);
        if (result_ranges.size() > 0)
        {
            optimal_node->cost = func::sum(result_ranges, [](const Range &result_range) {
                return result_range.cost;
            });
        }
        optimal_node->print("initial optimal");
    }

    shared_ptr<BaseNode> initial_node = make_shared<Node>();
    {
        initial_node->cost = 0;
        initial_node->value = 0;
        initial_node->cursor = 0;
        initial_node->bound();
        initial_node->print("initial node");
    }

    auto comp = [](const shared_ptr<BaseNode> &a, const shared_ptr<BaseNode> &b) {
        return a->cost_lower_bound > b->cost_lower_bound; // to make it a min heap
    };

    using NodeQueue = priority_queue<shared_ptr<BaseNode>, vector<shared_ptr<BaseNode>>, reference_wrapper<decltype(comp)>>;

    auto nodes = NodeQueue(std::ref(comp));
    nodes.push(initial_node);

    while (nodes.size() > 0)
    {
        auto node = nodes.top();
        nodes.pop();
        if (node == nullptr)
        {
            cerr << "weird nullptr of node" << endl;
            abort();
        }

        sw.pause(); // pause the timer for debug informaiton
        if (nodes.size() % 10000 == 0)
        {
            cout << "number of nodes: " << nodes.size() << ", current/optimal: " << node->cost_lower_bound << "/" << optimal_node->cost << endl;
        }
        if (sw.lap() > 200)
        {
            result_ranges.clear(); // cannot ensure the optimal
            return;
        }
        // auto desc = node->describe();
        // desc["t"] = t; // to avoid lag
        // desc["number of nodes"] = nodes.size();
        // desc["cost upper bound"] = optimal_node->cost;
        // report.push_back(desc);
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

        // after a better solution is found, the previous added node may have lower bound larger than optimal
        // so clear it immediately
        if (node->cost_lower_bound > optimal_node->cost)
        {
            nodes = decltype(nodes)(std::ref(comp)); // empty the nodes, it's bit of tricky because of the queue struct
        }
    }

    if (optimal_node->selected.size() > 0) // a solution better than greedy found
    {
        result_ranges.clear();
        for (auto selected : optimal_node->selected)
        {
            result_ranges.push_back(BaseNode::ranges[selected]);
        }
    }
}

BnbOptimizer::BnbOptimizer(double target_coverage) : Optimizer(target_coverage)
{
}

json BnbOptimizer::optimize(const Universe &universe, const Ranges &ranges, Ranges &result_ranges) const
{
    report.clear();
    struct Node : public BaseNode
    {
        vector<bool> visited;

        Node() : BaseNode()
        {
            visited.resize(universe->elements.size(), false);
        }

        shared_ptr<BaseNode> clone()
        {
            return make_shared<Node>(*this);
        }

        void update_value()
        {
            assert(universe->elements.size() == visited.size() && "visited should be initialized");
            for (const auto &element : ranges[cursor].elements)
            {
                if (visited[element.id] == 0)
                {
                    visited[element.id] = 1;
                    value += element.value;
                }
            }
        }

        void bound()
        {
            assert(universe->elements.size() == visited.size() && "visited should be initialized");
            cost_lower_bound = cost;
            double current_value = value;
            auto visited_copy = visited;
            for (int i = cursor; i < ranges.size(); ++i)
            {
                const auto &range = ranges[i];
                for (const auto &element : range.elements) // k
                {
                    if (visited_copy[element.id] == 0)
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
                        visited_copy[element.id] = 1;
                    }
                }
            }
            if (current_value < target_value)
            {
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

    branch_and_bound<Node>(universe, ranges, result_ranges, target_coverage, report);
    return report;
}

OnlineBnbOptimizer::OnlineBnbOptimizer(double target_coverage) : Optimizer(target_coverage)
{
}

json OnlineBnbOptimizer::optimize(const Universe &universe, const Ranges &ranges, Ranges &result_ranges) const
{
    report.clear();
    struct Node : BaseNode
    {
        list<Polygon> offcuts;

        shared_ptr<BaseNode> clone()
        {
            return make_shared<Node>(*this);
        }

        void update_value()
        {
            assert(cursor < ranges.size() && "cursor out of ranges!");
            // offcuts & covered
            offcuts = difference(offcuts, {ranges[cursor].product->polygon});
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
                    auto scene_offcuts = intersection(universe->roi->polygon, range.product->polygon);
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
    branch_and_bound<Node>(universe, ranges, result_ranges, target_coverage, report);
    return report;
}