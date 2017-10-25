#include "model.h"

using namespace std;

nlohmann::json to_json(const list<Polygon> &polys)
{
    auto ret = nlohmann::json();
    for (const auto &poly : polys)
    {
        ret.push_back(to_string(poly));
    }
    return ret;
}

nlohmann::json to_json(Model *m)
{
    auto ret = nlohmann::json();
    ret["polygon"] = m->s;
    ret["cells"] = m->cell_set;
    ret["offcuts"] = to_json(m->offcuts);
    return ret;
}

nlohmann::json to_json(AOI *aoi)
{
    return to_json((Model *)aoi);
}

nlohmann::json to_json(Scene *scene)
{
    auto ret = to_json((Model *)scene);
    ret["price"] = scene->price;
    return ret;
}

nlohmann::json to_json(const list<Scene *> &scenes)
{
    auto ret = nlohmann::json();
    for (auto scene : scenes)
    {
        ret.push_back(to_json(scene));
    }
    return ret;
}

Visualizer::Visualizer(AOI *aoi)
{
    jsn["aoi"] = to_json(aoi);
    stopwatch.restart();
}

void Visualizer::add_frame(Scene *selected, const list<Scene *> unselected)
{
    double elapsed = stopwatch.lap();
    string timestamp = to_string(elapsed);
    jsn["frames"][timestamp] = {{"selected", to_json(selected)}, {"unselected", to_json(unselected)}};
}

ostream &operator<<(ostream &os, const Visualizer &visualizer)
{
    os << visualizer.jsn;
}