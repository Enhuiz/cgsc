#include "model.h"

#include <iostream>
#include <fstream>
#include <set>

using namespace std;

bool operator==(const Element &a, const Element &b)
{
    return a.index == b.index;
}

void Range::update_cost()
{
    if (entity != nullptr)
        cost = entity->price;
}

void Range::update_value()
{
    value = func::sum(elements, [](const Element &e) {
        return e.value;
    });
}

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

    Polygon get_cell_polygon(const int &cid) const
    {
        return get_cell_polygon(cid_to_point(cid));
    }

    unordered_set<int> discretize(const Polygon &poly, bool keep_edge_cells) const
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

        auto ret = unordered_set<int>();
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

    int index_to_cid(int xi, int yi) const
    {
        return xi + (yi << 16);
    }

    Point cid_to_point(const int &cid) const
    {
        int xi = (uint16_t)cid & (~0);
        int yi = cid >> 16;
        return {xi * delta, yi * delta};
    }
};

Transformer::Transformer(const Entity &roi,
            const list<Entity> &records,
            double delta)
{
    universe.entity = &roi;
    ranges = func::map(records, [](const Entity &record) {
        auto range = Range();
        range.entity = &record;
        range.update_cost();
        return range;
    });
}

Geometric::Geometric(const Entity &roi, const list<Entity> &records, double delta)
    : Transformer(roi, records, delta)
{
    universe.value = area(universe.entity->poly);
    for (auto &range : ranges)
    {
        range.value = func::sum(intersection(range.entity->poly, universe.entity->poly), area);
    }
}

Discrete::Discrete(const Entity &roi, const list<Entity> &records, double delta)
    : Transformer(roi, records, delta)
{
    Discretizer discretizer{delta};

    { // initialize universe
        auto eids = discretizer.discretize(roi.poly, true);
        universe.elements.clear();
        for (auto eid : eids)
        {
            universe.elements.insert(Element{eid, 1});
        }
        universe.update_value();
    }

    { // initialize ranges
        auto roi_bounding_box = discretizer.bounding_box(roi.poly, floor_double, ceil_double);
        for (auto &range : ranges)
        {
            auto polys = intersection(range.entity->poly, roi_bounding_box);
            for (const auto &poly : polys)
            {
                auto eids = discretizer.discretize(poly, false);
                for (auto eid : eids)
                {
                    if (intersects(discretizer.get_cell_polygon(eid), roi.poly))
                    {
                        range.elements.insert(Element{eid, 1});
                    }
                }
            }
            range.update_value();
        }
    }
}

struct Cell
{
    Polygon poly;   // geometric shape of the cell
    list<Range *> owners; // the rectangles that cover the cell
};

Continuous::Continuous(const Entity &roi, const list<Entity> &records, double delta)
    : Transformer(roi, records, delta)
{
    auto cells = list<Cell>(); // cells currently got

    for (auto &range : ranges)
    {
        // for example, currently we have cell 1 and 2
        
        // ------------
        // | 1     /  |  
        // |      /   |
        // |     / 2  |
        // ------------
        
        // and a new product 
        
        //      -------------
        //     /           /
        //    ------------
        
        // the following process does this thing
        
        // ------------
        // | o     / o|  
        // |      ----|--------
        // |     /  i |      /
        // -----------------
        
        // i for inner, o for outer. The 2 outer and 1 inner have been updated, and get

        // ------------
        // | 1'    /2'|  
        // |      ----|--------
        // |     / 3' |      /
        // -----------------

        auto range_polys = intersection(range.entity->poly, roi.poly); // fetch the products region that is inside ROI
        auto new_inner_cells = list<Cell>();
        auto new_outer_cells = list<Cell>();
        for (const auto &range_poly : range_polys) // for each inside region, in most case there is only 1 region
        {
            for (auto it = cells.begin(); it != cells.end();) 
            {
                auto inners = list<Polygon>();
                auto outers = list<Polygon>();
                tie(inners, outers) = clip(it->poly, range_poly);
                if (inners.size() > 0) // the product intersects with the cell
                {
                    auto prev_owners = move(it->owners);    // record the owner of the cell
                    it = cells.erase(it);                   // discard the cell
                    for (auto outer : outers)               
                    {
                        new_outer_cells.push_back(Cell{outer, prev_owners}); 
                    }
                    prev_owners.push_back(&range);  // the inners are also covered by the product itself 
                    for (auto inner : inners)
                    {
                        new_inner_cells.push_back(Cell{inner, prev_owners});
                    }
                }
                else
                {
                    ++it;
                }
            }
        }
        cells.splice(cells.end(), new_inner_cells);
        cells.splice(cells.end(), new_outer_cells);

        // the following difference does this things:

        // from 
        //      -------------
        //     /  3'|      /
        //    ------------
        
        // to
        //      -------------
        //     /  3'|  4'  /
        //    ------------
        
        // so we get
        // ------------
        // | 1'    /2'|  
        // |      ----|--------
        // |     / 3' |  4'  /
        // -----------------

        range_polys = difference(range_polys,
                                 func::map(new_inner_cells, // get all polygon of new inner cells
                                           [](const Cell &cell) {
                                               return cell.poly;
                                           }));

        for (const auto &range_poly : range_polys)
        {
            cells.push_back(Cell{range_poly, {&range}});
        }
    }

    // calculate the value (i.e. area) of each cell
    auto area_map = map<list<Range *>, double>();
    for (const auto &cell : cells)
    {
        if (area_map.count(cell.owners) == 0)
        {
            area_map[cell.owners] = 0;
        }
        area_map[cell.owners] += area(cell.poly);
    }

    {
        int cell_index = 0; // index generator
        for (const auto &kv : area_map)
        {
            // update universe
            universe.elements.insert(Element{cell_index, kv.second});

            // update ranges
            for (auto range : kv.first)
            {
                range->elements.insert(Element{cell_index, kv.second});
            }

            ++cell_index;
        }
        // the following calculation add an imagenary cell that no one covers
        // to make sure the universe is complete
        double uncovered_area = area(roi.poly) - func::sum(universe.elements,
                                                           [](const Element &e) {
                                                               return e.value;
                                                           });
        universe.elements.insert(Element{cell_index, uncovered_area});
    }

    universe.update_value();
    for (auto &range : ranges)
    {
        range.update_value();
    }
}