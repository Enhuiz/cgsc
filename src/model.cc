#include "model.h"

using namespace std;

namespace discrete
{
static const auto floor_double = static_cast<double (*)(double)>(floor);
static const auto ceil_double = static_cast<double (*)(double)>(ceil);

struct Discretizer
{
    double delta;

    tuple<double, double, double, double> bounding_box_corner(const Polygon &poly,
                                                              const function<double(double)> &min_trunc,
                                                              const function<double(double)> &max_trunc) const
    {
        double minx, miny, maxx, maxy;
        minx = maxx = poly.begin()->x;
        miny = maxy = poly.begin()->y;
        for (const auto &p : poly)
        {
            minx = min(minx, p.x);
            miny = min(miny, p.y);
            maxx = max(maxx, p.x);
            maxy = max(maxy, p.y);
        }
        minx = min_trunc(minx / delta) * delta;
        miny = min_trunc(miny / delta) * delta;
        maxx = max_trunc(maxx / delta) * delta;
        maxy = max_trunc(maxy / delta) * delta;
        return make_tuple(minx, miny, maxx, maxy);
    }

    Polygon bounding_box(const Polygon &poly,
                         const function<double(double)> &min_trunc,
                         const function<double(double)> &max_trunc) const
    {
        double minx, miny, maxx, maxy;
        tie(minx, miny, maxx, maxy) = bounding_box_corner(poly, min_trunc, max_trunc);
        return {Point{minx, miny}, Point{maxx, miny}, Point{maxx, maxy}, Point{minx, maxy}};
    }

    Polygon get_cell_polygon(const Point &ll) const // lower left
    {
        double x = ll.x;
        double y = ll.y;
        return Polygon{{x, y},
                       {x + delta, y},
                       {x + delta, y + delta},
                       {x, y + delta}};
    }

    Polygon get_cell_polygon(const CID &cid) const
    {
        return get_cell_polygon(cid_to_point(cid));
    }

    CellSet discretize(const Polygon &poly, bool keep_edge_cells) const
    {
        using ConditionType = function<bool(const Point &)>;

        ConditionType discard_edge = [&poly, this](const Point &ll) {
            auto cell_poly = get_cell_polygon(ll);
            return all_of(cell_poly.begin(), cell_poly.end(), [&poly](const Point &p) {
                return !outside(p, poly); // it's OK for cell points on edges, while out is not acceptable.
            });
        };

        ConditionType keep_edge = [&poly, this](const Point &ll) {
            return intersects(get_cell_polygon(ll), poly);
        };

        ConditionType condition = keep_edge_cells ? keep_edge : discard_edge;

        double minx, miny, maxx, maxy;
        tie(minx, miny, maxx, maxy) = keep_edge_cells ? bounding_box_corner(poly, floor_double, ceil_double) : bounding_box_corner(poly, ceil_double, floor_double);

        int minxi = round(minx / delta);
        int minyi = round(miny / delta);
        int maxxi = round(maxx / delta);
        int maxyi = round(maxy / delta);

        CellSet ret;
        for (int i = minxi; i < maxxi; ++i)
        {
            for (int j = minyi; j < maxyi; ++j)
            {
                if (condition(Point{i * delta, j * delta}))
                {
                    ret.insert(index_to_cid(i, j));
                }
            }
        }
        return ret;
    }

    CID index_to_cid(int xi, int yi) const
    {
        return xi + ((CID)yi << 32);
    }

    Point cid_to_point(const CID &cid) const
    {
        int xi = cid & (~0);
        int yi = cid >> 32;
        return {xi * delta, yi * delta};
    }
};

void discretize_roi(ROI *roi, double delta)
{
    Discretizer discretizer{delta};
    roi->cell_set = discretizer.discretize(roi->poly, true);
}

// O(n m log m)
void discretize_scenes(const std::list<Scene *> &scenes, ROI *roi, double delta)
{
    Discretizer discretizer{delta};
    auto roi_bbox = discretizer.bounding_box(roi->poly, floor_double, ceil_double);
    for (auto scene : scenes) // n
    {
        auto polys = intersection(roi_bbox, scene->poly); // this indeed speed up the next instruction
        for (const auto &poly : polys)
        {
            auto cell_set = discretizer.discretize(poly, false);
            // find cid in the intersections
            for (auto cid : cell_set) // O(m)
            {
                if (intersects(discretizer.get_cell_polygon(cid), roi->poly)) // was O(logM), is O(1)
                {
                    scene->cell_set.insert(cid); // O(logm)
                }
            }
        }
    }
}

void remove_scenes_with_empty_cell_set(list<Scene *> &scenes)
{
    scenes.erase(remove_if(scenes.begin(),
                           scenes.end(),
                           [](const Scene *scene) {
                               return scene->cell_set.size() == 0;
                           }),
                 scenes.end());
};
}

namespace continuous
{

void remove_tiny_offcuts(list<Polygon> &offcuts, double delta)
{
    offcuts.erase(remove_if(offcuts.begin(),
                            offcuts.end(),
                            [delta](const Polygon &offcut) {
                                return area(offcut) < delta * delta;
                            }),
                  offcuts.end());
}

void remove_scenes_with_no_offcuts(list<Scene *> &scenes)
{
    scenes.erase(remove_if(scenes.begin(),
                           scenes.end(),
                           [](const Scene *scene) {
                               return scene->offcuts.size() == 0;
                           }),
                 scenes.end());
};

double area(const list<Polygon> &offcuts)
{
    double ret = 0;
    for (const auto &offcut : offcuts)
    {
        ret += area(offcut);
    }
    return ret;
};

void cut_roi(ROI *roi)
{
    if (convex(roi->poly))
    {
        roi->offcuts = list<Polygon>{roi->poly};
    }
    else
    {
        roi->offcuts = triangulate(roi->poly);
    }
}

void cut_scenes(const list<Scene *> &scenes, ROI *roi)
{
    for (auto scene : scenes)
    {
        if (convex(scene->poly))
        {
            scene->offcuts = list<Polygon>{scene->poly};
        }
        else
        {
            scene->offcuts = triangulate(scene->poly);
        }
        scene->offcuts = intersection(scene->offcuts, roi->offcuts);
    }
}

double calculate_coverage_ratio(ROI *roi, const list<Scene *> &scenes)
{
    double covered = 0;
    cut_roi(roi);
    for (auto i = scenes.begin(); i != scenes.end(); ++i)
    {
        cut_scenes(scenes, roi);
    }
    for (auto i = scenes.begin(); i != scenes.end(); ++i)
    {
        covered += area((*i)->offcuts);
        for (auto j = next(i); j != scenes.end(); ++j)
        {
            (*j)->offcuts = difference((*j)->offcuts, (*i)->offcuts);
        }
    }
    return covered / area(roi->poly);
}

nlohmann::json to_json(const list<Polygon> &polys)
{
    auto ret = nlohmann::json();
    for (const auto &poly : polys)
    {
        ret.push_back(to_string(poly));
    }
    return ret;
}

}