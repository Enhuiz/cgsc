#include "solution.h"

#include <algorithm>
#include <list>
#include <iostream>

using namespace std;

vector<Scene *> select_possible_scenes(AOI *aoi, const vector<Scene *> &scenes)
{
    vector<Scene *> ret;
    copy_if(scenes.begin(), scenes.end(), back_inserter(ret), [aoi](Scene *scene) {
        return intersects(aoi->poly, scene->poly);
    });
    return ret;
}

void discretize_aoi(const Discretizer &discretizer, AOI *aoi)
{
    aoi->cell_set = discretizer.discretize(aoi->poly, true);
}

// O(nmlogm)
void discretize_scenes(const Discretizer &discretizer, AOI *aoi, const vector<Scene *> &scenes)
{
    auto &acs = aoi->cell_set;
    for (auto scene : scenes) // n
    {
        scene->cell_set = discretizer.discretize(scene->poly, false); //mlogm

        // intersection
        CellSet new_cs;
        auto &cs = scene->cell_set;
        set_intersection(cs.begin(), cs.end(), acs.begin(), acs.end(), inserter(new_cs, new_cs.begin()));
        cs = new_cs;
    }
}

// O(n^2mlogm)
vector<Scene *> select_approx_optimal_scenes(const AOI *aoi, const vector<Scene *> &scenes)
{
    list<Scene *> possible_scenes(scenes.begin(), scenes.end());
    vector<Scene *> result_scenes;
    int covered = 0;
    int num_aoi_cells = aoi->cell_set.size();

    auto remove_scenes_with_empty_cellset = [](list<Scene *> &scenes) {
        scenes.erase(remove_if(scenes.begin(),
                               scenes.end(),
                               [](const Scene *scene) {
                                   return scene->cell_set.size() == 0;
                               }),
                     scenes.end());
    };

    remove_scenes_with_empty_cellset(possible_scenes);
    while (covered < num_aoi_cells && possible_scenes.size() > 0) // n
    {
        auto it = min_element(possible_scenes.begin(), possible_scenes.end(), [](const Scene *a, const Scene *b) { // n
            return a->price / a->cell_set.size() < b->price / b->cell_set.size();
        });
        // remove the selected
        auto scene = *it;
        possible_scenes.erase(it);
        // remove cells from the left possible scenes
        for (auto &possible_scene : possible_scenes) // n
        {
            for (const auto &cell : scene->cell_set) // m
            {
                possible_scene->cell_set.erase(cell); // logm
            }
        }
        covered += scene->cell_set.size();
        result_scenes.push_back(scene);
        remove_scenes_with_empty_cellset(possible_scenes);
    }
    return result_scenes;
}
