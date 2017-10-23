#include "experiment.h"

#include <iostream>

#include "global.h"

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
                if (aoi->poly.front() == aoi->poly.back()) { // use non-closed representation 
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
                if (scene->poly.front() == scene->poly.back()) {
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

struct Analyser
{
    bool polygon_enabled;
    bool cell_enabled;
    bool offcuts_enabled;

    nlohmann::json get_scenes_report(const list<Scene *> scenes) const
    {
        nlohmann::json report;
        for (auto scene : scenes)
        {
            report.push_back(get_scene_report(scene));
        }
        return report;
    }

    nlohmann::json get_scene_report(const Scene *scene) const
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
        if (offcuts_enabled)
        {
            ret["offcuts"] = get_offcuts_report(scene->offcuts);
        }
        ret["price"] = scene->price;
        return ret;
    };

    nlohmann::json get_aoi_report(const AOI *aoi) const
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
        if (offcuts_enabled)
        {
            ret["offcuts"] = get_offcuts_report(aoi->offcuts);
        }
        return ret;
    };

    nlohmann::json get_offcuts_report(const list<Triangle> &offcuts) const
    {
        auto report = nlohmann::json();

        for (const auto &offcut : offcuts)
        {
            report.push_back(to_string(offcut));
        }
        return report;
    };

    double calculate_coverage_ratio(const AOI *aoi, const list<Scene *> &scenes) const
    {
        int num_covered_cells = accumulate(scenes.begin(), scenes.end(), 0, [](int acc, Scene *scene) {
            return acc + scene->cell_set.size();
        });
        return num_covered_cells * 1.0 / aoi->cell_set.size();
    };
};

nlohmann::json discrete_query(AOI *aoi, const list<Scene *> &scenes, double delta)
{
    using namespace discrete;

    nlohmann::json report;
    Analyser analyser{true, true, false};

    timer.begin("t1");
    auto possible_scenes = select_possible_scenes(aoi, scenes);
    discretize_aoi(aoi, delta);
    discretize_scenes(possible_scenes, aoi, delta);
    double t1 = timer.end();

    report["possible_scenes"] = analyser.get_scenes_report(possible_scenes);
    report["aoi"] = analyser.get_aoi_report(aoi);

    timer.begin("t2");
    auto result_scenes = select_approx_optimal_scenes(aoi, possible_scenes);
    double t2 = timer.end();

    report["result_scenes"] = analyser.get_scenes_report(result_scenes);
    report["coverage_ratio"] = analyser.calculate_coverage_ratio(aoi, result_scenes);
    report["t1"] = t1;
    report["t2"] = t2;
    report["delta"] = delta; // for run.py

    // release memory
    for (auto scene : scenes)
    {
        scene->cell_set.clear();
    }
    aoi->cell_set.clear();

    return report;
}

nlohmann::json continuous_query(AOI *aoi, const list<Scene *> &scenes)
{
    using namespace continuous;

    nlohmann::json report;
    Analyser analyser{false, false, false};

    timer.begin("t1");
    auto possible_scenes = select_possible_scenes(aoi, scenes);
    double t1 = timer.end();

    report["aoi"] = analyser.get_aoi_report(aoi);
    report["possible_scenes"] = analyser.get_scenes_report(possible_scenes);

    timer.begin("t2");
    auto result_scenes = select_approx_optimal_scenes(aoi, possible_scenes);
    double t2 = timer.end();

    report["result_scenes"] = analyser.get_scenes_report(result_scenes);
    report["coverage_ratio"] = 1;
    report["timer"] = {{"t1", t1}, {"t2", t2}};

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
    auto reports = nlohmann::json();

    timer.begin("loading");
    const auto loader = Loader(aois_path, scenes_path);
    const auto aois = loader.get_aois();
    const auto scenes = loader.get_scenes();
    timer.end();

    for (auto &aoi : aois)
    {
        reports.push_back(discrete_query(aoi, scenes, delta));
        // reports.push_back(continuous_query(aoi, scenes));
    }

    return reports;
}