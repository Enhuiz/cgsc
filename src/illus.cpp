// #include <iostream>
// #include <string>
// #include <vector>
// #include <fstream>

// #include <boost/program_options.hpp>

// #include "json.hpp"

// #include "cgsc/solver/greedy.h"
// #include "cgsc/utils/data.h"
// #include "cgsc/utils/result.h"
// #include "cgsc/utils/timestamp.h"

// using namespace boost::program_options;
// using namespace std;

// using namespace cgsc::model;
// using namespace cgsc::utils;
// using namespace cgsc::solver;

// int main()
// {
//     nlohmann::json jobj;

//     Timestamp::Begin("loading data");
//     auto data = Data("../data/illustration/input/scenes.csv", "../data/illustration/input/aois.csv");
//     Timestamp::End();

//     auto greedy = Greedy();

//     const auto &aois = data.getAOIs();
//     for (int i = 0; i < aois.size(); ++i)
//     {
//         const auto &aoi = *aois[i];

//         Timestamp::Begin("aoi" + to_string(i) + "::t1");
//         auto possibleScenes = greedy.selectPossibleScenes(aoi, data.getScenes());
//         double t1 = Timestamp::End();

//         Timestamp::Begin("aoi" + to_string(i) + "::t2");
//         // copy aoi and scenes so that we can modify it
//         auto clonedPossibleScenes = vector<shared_ptr<Scene>>();
//         for (const auto &possibleScene : possibleScenes)
//         {
//             clonedPossibleScenes.push_back(make_shared<Scene>(*possibleScene));
//         }
//         auto resultScenes = greedy.optimize(aoi, clonedPossibleScenes);
//         double t2 = Timestamp::End();

//         auto result = Result();

//         result.addAOI(aoi, true);
//         result.addPossibleScenes(possibleScenes, true);
//         result.addResultScense(resultScenes, true);
//         result.addTotalPrice(resultScenes);
//         result.addCoverageArea(aoi, resultScenes);

//         jobj.push_back(result.toJSON());
//     }

//     ofstream ofs("../data/illustration/output/result.json");
//     ofs << jobj << endl;

//     return 0;
// }
