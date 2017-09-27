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

void experiment(const std::string &scenesPath, const std::string &aoisPath, const std::string &outPath)
{
    nlohmann::json jobj;
    auto timestamp = Timestamp();

    timestamp.begin("loading data");
    auto data = Data(scenesPath, aoisPath);
    timestamp.end();

    auto greedy = Greedy();

    const auto &aois = data.getAOIs();
    for (int i = 0; i < aois.size(); ++i)
    {
        const auto &aoi = aois[i];

        timestamp.begin("aoi" + to_string(i) + "::t1");
        auto possibleScenes = greedy.selectPossibleScenes(*aoi, data.getScenes());
        double t1 = timestamp.end();

        timestamp.begin("aoi" + to_string(i) + "::t2");
        auto resultScenes = greedy.optimize(*aoi, possibleScenes);
        double t2 = timestamp.end();

        auto resultJobj = Result(*aoi, possibleScenes, resultScenes).toJSON();

        resultJobj["t1"] = t1;
        resultJobj["t2"] = t2;

        jobj.push_back(resultJobj);
    }

    ofstream ofs(outPath);
    ofs << jobj << endl;
}

int main(int argc, char *argv[])
{
    auto desc = options_description("Options");
    desc.add_options()("aoi-path,a", value<string>(), "source file of aoi")("scenes-path,s", value<string>(), "source file of scenes")("output-path,o", value<string>(), "output path");

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

    if (vm.count("aoi-path") && vm.count("scenes-path") && vm.count("output-path"))
    {
        experiment(vm["scenes-path"].as<string>(), vm["aoi-path"].as<string>(), vm["output-path"].as<string>());
    }
    else
    {
        cout << desc << endl;
    }

    return 0;
}
