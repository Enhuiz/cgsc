#include "experiment.h"

#include <iostream>

#include "csv.hpp"
#include "global.h"
#include "solution.h"

using namespace std;
using namespace nlohmann;

struct Loader
{

    std::list<std::unique_ptr<Entity>> rois;
    std::list<std::unique_ptr<Entity>> scenes;

    Loader(const string &rois_path, const string &scenes_path)
    {
        { // load roi
            io::CSVReader<1, io::trim_chars<' '>, io::double_quote_escape<',', '\"'>> in(rois_path);
            in.read_header(io::ignore_extra_column, "Polygon");
            string poly_s;
            while (in.read_row(poly_s))
            {
                auto roi = unique_ptr<Entity>(new Entity);
                roi->s = poly_s;
                roi->poly = parse_polygon(poly_s);
                if (roi->poly.front() == roi->poly.back())
                { // use non-closed representation
                    roi->poly.pop_back();
                }
                rois.push_back(move(roi));
            }
        }
        { // load scenes
            io::CSVReader<2, io::trim_chars<' '>, io::double_quote_escape<',', '\"'>> in(scenes_path);
            in.read_header(io::ignore_extra_column, "Polygon", "Price");
            string poly_s;
            double price;
            while (in.read_row(poly_s, price))
            {
                auto scene = unique_ptr<Entity>(new Entity);
                scene->s = poly_s;
                scene->poly = parse_polygon(poly_s);
                if (scene->poly.front() == scene->poly.back())
                {
                    scene->poly.pop_back();
                }
                scene->price = price;
                scenes.push_back(move(scene));
            }
        }
    }

    list<Entity *> get_scenes() const
    {
        return func::map(scenes, [](const unique_ptr<Entity> &p) {
            return p.get();
        });
    }

    list<Entity *> get_rois() const
    {
        return func::map(rois, [](const unique_ptr<Entity> &p) {
            return p.get();
        });
    }
};

// void append_results_to_report(nlohmann::json &report, Entity *roi, const list<Entity *> &possible_scenes, const list<Entity *> &selected_scenes)
// {
//     report["price"] = calculate_total_price(selected_scenes);
//     report["number_of_possible_scenes"] = possible_scenes.size();
//     report["number_of_selected_scenes"] = selected_scenes.size();
//     report["coverage_ratio"] = calculate_coverage_ratio(roi, selected_scenes);
// }

// nlohmann::json query(Entity *roi, const list<Entity *> &scenes, double target_coverage, double delta, Optimizer optimize)
// {
//     g_report.clear();
//     // Stopwatch sw;
//     // sw.restart();
//     auto possible_scenes = select_possible_scenes(roi, scenes);
//     // g_report["t_find_possible_scenes"] = sw.lap();

//     auto selected_scenes = optimize(roi, possible_scenes, target_coverage, delta);

//     if (selected_scenes.size() == 0) // no result satisfy the target coverage ratio
//         return {};

//     append_results_to_report(g_report, roi, possible_scenes, selected_scenes);

//     logger << "[optimal]" << endl;
//     logger << "price: " << g_report["price"] << endl;
//     logger << "covered: " << g_report["coverage_ratio"] << endl;
//     logger << "scenes: " << g_report["number_of_selected_scenes"] << endl;

//     return g_report;
// }

struct Executor
{
    const list<Entity *> &scenes;
    double target_coverage;
    double delta;
    json report;

    Executor(const list<Entity *> &scenes,
             double target_coverage,
             double delta) : scenes(scenes),
                             target_coverage(target_coverage),
                             delta(delta)
    {
    }

    template <class... Solvers>
    void query(Entity *roi)
    {
        int _[] = {0, (query_impl<Solvers>(roi), 0)...};
    }

    template <class Solver>
    void query_impl(Entity *roi)
    {
        auto tag = Solver::tag();
        logger.push_namespace(tag);
        {
            Stopwatch sw;
            auto solver = Solver(roi, scenes, target_coverage, delta);
            // logger << "end after " << sw.lap() << " s" << endl;
            report[tag].push_back(solver.report);
        }
        logger.pop_namespace();
    }
};

json experiment(const string &rois_path, const string &scenes_path, double target_coverage, double delta)
{
    const auto loader = Loader(rois_path, scenes_path);
    const auto rois = loader.get_rois();
    const auto scenes = loader.get_scenes();

    auto executor = Executor(scenes, target_coverage, delta);

    {
        int i = 0;
        for (auto roi : rois)
        {
            logger.push_namespace(to_string(i));
            executor.query<
                Solver<Continuous, Optimizer>>(roi);
            // execute("dg", roi, discrete::greedy::optimize);
            // execute("cg", roi, continuous::greedy::optimize);
            // execute("sg", roi, semantical::greedy::optimize);
            // execute("cb", roi, continuous::branch_and_bound::optimize);
            // execute("sb", roi, semantical::branch_and_bound::optimize);
            logger.pop_namespace();
            ++i;
        }
    }

    return executor.report;
}
