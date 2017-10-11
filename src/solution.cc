#include "solution.h"

std::vector<Scene *> select_possible_scenes(AOI *aoi, const std::vector<Scene *> &scenes)
{
    std::vector<Scene *> ret;
    copy_if(scenes.begin(), scenes.end(), back_inserter(ret), [aoi](Scene *scene) {
        return intersects(*aoi, *scene);
    });
    return ret;
}

AOI *discretize_aoi(const Discretizer &discretizer, AOI *aoi)
{
    aoi->cell_set = discretizer.discretize(aoi->poly);
}

// O(nmlogm)
std::vector<Scene *> discretize_scenes(const Discretizer &discretizer, AOI *aoi, const std::vector<Scene *> &scenes)
{
    auto &a_cs = aoi->cell_set;
    for (auto scene : scenes) // n
    {
        scene->cell_set = discretizer.discretize(scene->poly); //mlogm
        // remove cids not in a_cs
        auto &s_cs = scene->cell_set;
        auto iter = remove_if(s_cs.begin(), s_cs.end(), [&a_cs](const CID &cid) { //m
            return a_cs.count(cid) == 0;
        });
        s_cs.earse(iter, s_cs.end());
    }
    return scenes;
}

// O(n^2mlogm)
std::vector<Scene *> select_approx_optimal_scenes(const AOI *aoi, const std::vector<Scene *> &scenes)
{

    list<Scene *> possible_scenes(scenes.begin(), scenes.end());
    list<Scene *> result_scenes;

    int covered = 0;
    int num_possible_scenes = possible_scenes.size();
    int num_aoi_cells = aoi->cell_set.size();

    while (covered < num_aoi_cells && result_scenes.size() < num_possible_scenes) // n
    {
        // remove empty cell_set, n
        auto iter = remove_if(possible_scenes.begin(), possible_scenes.end(), [](const Scene *scene) {
            return scene->cell_set.size() == 0;
        });
        possible_scenes.earse(iter, possible_scenes.end());
        // n
        auto it = min_element(possible_scenes.begin(), possible_scenes.end(), [](const auto &a, const auto &b) {
            return a->price / a->cell_set.size() < b->price / b->cell_set.size();
        });
        // remove the selected
        auto scene = *it;
        possible_scenes.erase(it);
        // remove cells from the left possible scenes
        for (auto &ps : possible_scenes) // n
        {
            for (const auto &cell : scene->cells) // m
            {
                ps->cells.erase(cell); // logm
            }
        }
        covered += scene->cells.size();
        result_scenes.push_back(scene);
    }
    return result_scenes;
}
