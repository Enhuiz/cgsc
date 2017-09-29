#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <boost/program_options.hpp>

#include "json.hpp"

#include "cgsc/solver/greedy.h"
#include "cgsc/utils/data.h"
#include "cgsc/utils/result.h"
#include "cgsc/utils/timestamp.h"

using namespace boost::program_options;
using namespace std;

using namespace cgsc::model;
using namespace cgsc::utils;
using namespace cgsc::solver;

void experiment(double delta, const std::string &scenesPath, const std::string &aoisPath, const std::string &outPath)
{
    vector<shared_ptr<const Scene>> scenes;

    auto data = Data(scenesPath, aoisPath);

    { // preprocess
        Timestamp::Begin("loading data");
        auto rawScenes = data.cloneScenes();
        Timestamp::End();

        Timestamp::Begin("scenes preprocessing");
        for (auto &scene : rawScenes)
        {
            scene->updateGrids(delta);
            scenes.push_back(shared_ptr<const Scene>(scene)); // lock to avoid modification
        }
        Timestamp::End();
    }

    auto jobj = nlohmann::json();
    
    { // query
        auto greedy = Greedy();
        auto aois = data.cloneAOIs();

        for (int i = 0; i < aois.size(); ++i)
        {
            auto &aoi = *aois[i];

            Timestamp::Begin("aoi" + to_string(i) + "::t1");
            auto possibleScenes = greedy.selectPossibleScenes(aoi, scenes);
            double t1 = Timestamp::End();

            Timestamp::Begin("aoi" + to_string(i) + "::t2");
            aoi.updateGrids(delta); // calculate aoi for each query here, rather than during preprocessing

            auto resultScenes = greedy.optimize(aoi, scenes);
            double t2 = Timestamp::End();

            auto result = Result();

            result.addTotalPrice(resultScenes);
            result.addCoverageRatio(aoi, resultScenes);
            result.addJSON("timestamp", {{"t1", t1}, {"t2", t2}});

            jobj.push_back(result.toJSON());
        }
    }

    { // output result
        ofstream ofs(outPath);
        ofs << jobj << endl;
    }
}

int main(int argc, char *argv[])
{
    auto desc = options_description("Options");
    desc.add_options()("aoi-path,a", value<string>(), "source file of aoi")("scenes-path,s", value<string>(), "source file of scenes")("output-path,o", value<string>(), "output path")("delta,d", value<double>(), "grid length");

    variables_map vm;
    try
    {
        store(parse_command_line(argc, argv, desc), vm);
        notify(vm);
    }
    catch (...)
    {
        cout << desc << endl;
        return 0;
    }

    if (vm.count("delta") && vm.count("aoi-path") && vm.count("scenes-path") && vm.count("output-path"))
    {
        experiment(vm["delta"].as<double>(), vm["scenes-path"].as<string>(), vm["aoi-path"].as<string>(), vm["output-path"].as<string>());
    }
    else
    {
        cout << desc << endl;
    }

    return 0;
}
