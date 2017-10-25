#include "experiment.h"

#include <iostream>

#include "csv.hpp"
#include "global.h"
#include "solution.h"

using namespace std;

struct Loader
{

    std::list<std::unique_ptr<AOI>> aois;
    std::list<std::unique_ptr<Scene>> scenes;

    Loader(const string &aois_path, const string &scenes_path)
    {
        { // load aoi
            io::CSVReader<1, io::trim_chars<' '>, io::double_quote_escape<',', '\"'>> in(aois_path);
            in.read_header(io::ignore_extra_column, "Polygon");
            string poly_s;
            while (in.read_row(poly_s))
            {
                auto aoi = unique_ptr<AOI>(new AOI);
                aoi->s = poly_s;
                aoi->poly = parse_polygon(poly_s);
                if (aoi->poly.front() == aoi->poly.back())
                { // use non-closed representation
                    aoi->poly.pop_back();
                }
                aois.push_back(move(aoi));
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

    list<AOI *> get_aois() const
    {
        list<AOI *> ret;
        for (const auto &ptr : aois)
            ret.push_back(ptr.get());
        return ret;
    }
};

double calculate_discrete_coverage_ratio(const AOI *aoi, const list<Scene *> &scenes)
{
    int num_covered_cells = accumulate(scenes.begin(), scenes.end(), 0, [](int acc, Scene *scene) {
        return acc + scene->cell_set.size();
    });
    return num_covered_cells * 1.0 / aoi->cell_set.size();
};

double calculate_continuous_coverage_ratio(const AOI *aoi, const list<Scene *> &scenes)
{
    double area_of_offcuts = accumulate(scenes.begin(), scenes.end(), 0.0, [](double acc, Scene *scene) {
        return acc + continuous::area(scene);
    });
    return area_of_offcuts / continuous::area(aoi);
}

double calculate_total_price(const list<Scene *> &scenes)
{
    return accumulate(scenes.begin(), scenes.end(), 0.0, [](double acc, Scene *scene) {
        return acc + scene->price;
    });
}

nlohmann::json discrete_query(AOI *aoi, const list<Scene *> &scenes, double delta)
{
    using namespace discrete;

    nlohmann::json report;

    timer.begin("t1");
    auto possible_scenes = select_possible_scenes(aoi, scenes);
    discretize_aoi(aoi, delta);
    discretize_scenes(possible_scenes, aoi, delta);
    double t1 = timer.end();

    timer.begin("t2");
    auto result_scenes = select_approx_optimal_scenes(aoi, possible_scenes);
    double t2 = timer.end();

    report["t1"] = t1;
    report["t2"] = t2;
    report["price"] = calculate_total_price(result_scenes);
    report["coverage_ratio"] = calculate_discrete_coverage_ratio(aoi, result_scenes);
    report["delta"] = delta; // for run.py

    // release memory
    for (auto scene : scenes)
    {
        scene->cell_set.clear();
    }
    aoi->cell_set.clear();

    return report;
}

nlohmann::json continuous_query(AOI *aoi, const list<Scene *> &scenes, double delta)
{
    using namespace continuous;

    nlohmann::json report;

    timer.begin("t1");
    auto possible_scenes = select_possible_scenes(aoi, scenes);
    cut_aoi(aoi, delta);
    cut_scenes(possible_scenes, aoi, delta);
    double t1 = timer.end();

    timer.begin("t2");
    auto result_scenes = select_approx_optimal_scenes(aoi, possible_scenes, delta);
    double t2 = timer.end();

    report["t1"] = t1;
    report["t2"] = t2;
    report["price"] = calculate_total_price(result_scenes);
    report["coverage_ratio"] = calculate_continuous_coverage_ratio(aoi, result_scenes);
    report["delta"] = delta; // for run.py

    // release memory
    for (auto scene : scenes)
    {
        scene->offcuts.clear();
    }
    aoi->offcuts.clear();

    return report;
}

nlohmann::json experiment(const string &aois_path, const string &scenes_path, double delta)
{
    auto discrete_reports = nlohmann::json();
    auto continuous_reports = nlohmann::json();

    timer.begin("loading");
    const auto loader = Loader(aois_path, scenes_path);
    const auto aois = loader.get_aois();
    const auto scenes = loader.get_scenes();
    timer.end();

    int cnt = 0;
    for (auto &aoi : aois)
    {
        logger.push_namespace("d" + to_string(cnt));
        discrete_reports.push_back(discrete_query(aoi, scenes, delta));
        logger.pop_namespace();

        logger.push_namespace("c" + to_string(cnt));
        continuous_reports.push_back(continuous_query(aoi, scenes, delta));
        logger.pop_namespace();
        ++cnt;
    }

    return {{"discrete", discrete_reports}, {"continuous", continuous_reports}};
}
