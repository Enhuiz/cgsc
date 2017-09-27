#include <iostream>
#include <string>
#include <vector>

#include "cgsc/model/polygon.h"
#include "cgsc/model/grid.h"
#include "cgsc/model/aoi.h"
#include "cgsc/model/scene.h"
#include "cgsc/solver/greedy.h"
#include "cgsc/utils/data.h"
#include "cgsc/utils/result.h"
#include "cgsc/utils/timestamp.h"

using namespace cgsc::model;
using namespace cgsc::utils;
using namespace cgsc::solver;

using namespace std;

int main()
{
    auto timestamp = Timestamp();
    
    timestamp.add("BEGIN data load");
    auto data = Data("../../data/input/scenes_small.csv", "../../data/input/aois.csv");
    timestamp.add("END data load");
    
    auto greedy = Greedy();
    string path = "../../data/output/";

    {
        const auto &aois = data.getAOIs();

        for (int i = 0; i < aois.size(); ++i)
        {
            const auto &aoi = aois[i];

            timestamp.add("BEGIN select possible scenes " + to_string(i));
            auto possibleScenes = greedy.selectPossibleScenes(*aoi, data.getScenes());
            timestamp.add("END select possible scenes " + to_string(i));

            timestamp.add("BEGIN optimize " + to_string(i));
            auto resultScenes = greedy.optimize(*aoi, possibleScenes);
            timestamp.add("END optimize " + to_string(i));

            auto result = Result(*aoi, possibleScenes, resultScenes);
            result.save(path + "result_" + to_string(i) + ".json");
        }
    }

    timestamp.save(path + "timestamp.json");

    return 0;
}
