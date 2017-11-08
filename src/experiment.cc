#include "experiment.h"

#include <iostream>

#include "csv.hpp"
#include "global.h"
#include "solution.h"

using namespace std;

struct Loader
{

    std::list<std::unique_ptr<ROI>> rois;
    std::list<std::unique_ptr<Scene>> scenes;

    Loader(const string &rois_path, const string &scenes_path)
    {
        { // load roi
            io::CSVReader<1, io::trim_chars<' '>, io::double_quote_escape<',', '\"'>> in(rois_path);
            in.read_header(io::ignore_extra_column, "Polygon");
            string poly_s;
            while (in.read_row(poly_s))
            {
                auto roi = unique_ptr<ROI>(new ROI);
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
                auto scene = unique_ptr<Scene>(new Scene);
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

    list<Scene *> get_scenes() const
    {
        list<Scene *> ret;
        for (const auto &ptr : scenes)
            ret.push_back(ptr.get());
        return ret;
    }

    list<ROI *> get_rois() const
    {
        list<ROI *> ret;
        for (const auto &ptr : rois)
            ret.push_back(ptr.get());
        return ret;
    }
};

double calculate_total_price(const list<Scene *> &scenes)
{
    return accumulate(scenes.begin(), scenes.end(), 0.0, [](double acc, Scene *scene) {
        return acc + scene->price;
    });
}

// nlohmann::json generate_report(ROI *roi, const list<Scene *> &possible_scenes, const list<Scene *> &selected_scenes)
// {
//     nlohmann::json report;

//     auto polygon_to_json = [](const Model *model) { return model->s; };
//     auto cell_set_to_json = [](const Model *model) { return nlohmann::json(model->cell_set); };
//     auto offcuts_to_json = [](const Model *model) { return to_json(model->offcuts); };
//     auto model_to_json = [polygon_to_json, cell_set_to_json, offcuts_to_json](const Model *model) {
//         nlohmann::json report;
//         report["polygon"] = polygon_to_json(model);
//         report["cell_set"] = cell_set_to_json(model);
//         report["offcuts"] = offcuts_to_json(model);
//         return report;
//     };

//     report["roi"] = model_to_json(roi);
//     report["possible_scenes"] = functional::map(possible_scenes, model_to_json);
//     report["selected_scenes"] = functional::map(selected_scenes, model_to_json);

//     return report;
// }

void append_results_to_report(nlohmann::json &report, ROI *roi, const list<Scene *> &possible_scenes, const list<Scene *> &selected_scenes)
{
    report["price"] = calculate_total_price(selected_scenes);
    report["number_of_possible_scenes"] = possible_scenes.size();
    report["number_of_selected_scenes"] = selected_scenes.size();
    report["coverage_ratio"] = continuous::calculate_coverage_ratio(roi, selected_scenes);
}

using Optimizer = function<list<Scene *>(ROI *, list<Scene *>, double, double)>;

nlohmann::json query(ROI *roi, const list<Scene *> &scenes, double target_coverage_ratio, double delta, Optimizer optimize)
{
    g_report.clear();
    // Stopwatch sw;
    // sw.restart();
    auto possible_scenes = select_possible_scenes(roi, scenes);
    // g_report["t_find_possible_scenes"] = sw.lap();

    auto selected_scenes = optimize(roi, possible_scenes, target_coverage_ratio, delta);

    if (selected_scenes.size() == 0) // no result satisfy the target coverage ratio
        return {};

    append_results_to_report(g_report, roi, possible_scenes, selected_scenes);

    logger << "[optimal]" << endl;
    logger << "price: " << g_report["price"] << endl;
    logger << "covered: " << g_report["coverage_ratio"] << endl;
    logger << "scenes: " << g_report["number_of_selected_scenes"] << endl;

    return g_report;
}

nlohmann::json experiment(const string &rois_path, const string &scenes_path, double target_coverage_ratio, double delta)
{
    const auto loader = Loader(rois_path, scenes_path);
    const auto rois = loader.get_rois();
    const auto scenes = loader.get_scenes();

    auto reports = nlohmann::json();

    auto execute = [&reports, &scenes, delta, target_coverage_ratio](const string &tag, ROI *roi, Optimizer optimizer) {
        Stopwatch sw;
        logger.push_namespace(tag);
        sw.restart();
        reports[tag].push_back(query(roi, scenes, target_coverage_ratio, delta, optimizer));
        logger << "end after " << sw.lap() << " s" << endl;
        logger.pop_namespace();
    };

    {
        int i = 0;
        for (auto roi: rois)
        {
            logger.push_namespace(to_string(i));
            execute("dg", roi, discrete::greedy::optimize);
            execute("cg", roi, continuous::greedy::optimize);
            execute("cb", roi, continuous::branch_and_bound::DFS::optimize);
            logger.pop_namespace();
            ++i;
        }
    }

    return reports;
}
