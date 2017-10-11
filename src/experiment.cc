#include "experiment.h"

#include "global.h"

#include <iostream>

using namespace std;

Loader::Loader(const string &aois_path, const string &scenes_path)
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
            scene->price = price;
            scenes.push_back(move(scene));
        }
    }
}

vector<Scene *> Loader::get_scenes() const
{
    vector<Scene *> ret;
    ret.reserve(scenes.size());
    for (const auto &ptr : scenes)
        ret.push_back(ptr.get());
    return ret;
}

vector<AOI *> Loader::get_aois() const
{
    vector<AOI *> ret;
    ret.reserve(aois.size());
    for (const auto &ptr : aois)
        ret.push_back(ptr.get());
    return ret;
}

nlohmann::json Analysor::get_scenes_report(const vector<Scene *> scenes) const
{
    nlohmann::json report;
    for (auto scene : scenes)
    {
        report.push_back(get_scene_report(scene));
    }
    return report;
}

nlohmann::json Analysor::get_scene_report(const Scene *scene) const
{
    nlohmann::json ret;
    if (polygon_enabled)
    {
        ret["polygon"] = scene->s;
    }
    if (cell_enabled)
    {
        ret["cells"] = scene->cell_set;
    }
    ret["price"] = scene->price;
    return ret;
};

nlohmann::json Analysor::get_aoi_report(const AOI *aoi) const
{
    nlohmann::json ret;
    if (polygon_enabled)
    {
        ret["polygon"] = aoi->s;
    }
    if (cell_enabled)
    {
        ret["cells"] = aoi->cell_set;
    }
    return ret;
};

double Analysor::calculate_coverage_ratio(const AOI *aoi, const vector<Scene *> &scenes) const
{
    int num_covered_cells = accumulate(scenes.begin(), scenes.end(), 0, [](int acc, Scene *scene) {
        return acc + scene->cell_set.size();
    });
    return num_covered_cells * 1.0 / aoi->cell_set.size();
};

nlohmann::json query(AOI *aoi, const vector<Scene *> &scenes, double delta)
{
    nlohmann::json report;
    Analysor analysor{false, false};
    auto discretizer = Discretizer{delta};

    timer.begin("t1");
    auto possible_scenes = select_possible_scenes(aoi, scenes);
    discretize_aoi(discretizer, aoi);
    discretize_scenes(discretizer, aoi, possible_scenes);
    double t1 = timer.end();

    report["aoi"] = analysor.get_aoi_report(aoi);
    report["possible_scenes"] = analysor.get_scenes_report(possible_scenes);

    timer.begin("t2");
    auto result_scenes = select_approx_optimal_scenes(aoi, possible_scenes);
    double t2 = timer.end();

    report["result_scenes"] = analysor.get_scenes_report(result_scenes);
    report["coverage_ratio"] = analysor.calculate_coverage_ratio(aoi, result_scenes);
    report["timer"] = {{"t1", t1}, {"t2", t2}};
    report["delta"] = delta;

    return report;
}

nlohmann::json experiment(const string &aois_path, const string &scenes_path, double delta)
{
    auto reports = nlohmann::json();

    timer.begin("loading");
    const auto loader = Loader(aois_path, scenes_path);
    const auto aois = loader.get_aois();
    const auto scenes = loader.get_scenes();
    timer.end();

    for (auto &aoi : aois)
    {
        reports.push_back(query(aoi, scenes, delta));
    }

    return reports;
}