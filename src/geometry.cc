#include "geometry.h"

#include <algorithm>
#include <functional>
#include <sstream>
#include <iostream>

using namespace std;

Vector2 Vector2::operator+(const Vector2 &other) const
{
    Vector2 ret;
    ret.x = x + other.x;
    ret.y = y + other.y;
    return ret;
}

Vector2 Vector2::operator-(const Vector2 &other) const
{
    Vector2 ret;
    ret.x = x - other.x;
    ret.y = y - other.y;
    return ret;
}

double cross_product(const Vector2 &a, const Vector2 &b)
{
    return a.x * b.y - a.y * b.x;
}

string to_string(const Vector2 &v)
{
    return "[" + std::to_string(v.x) + ", " + std::to_string(v.y) + "]";
}

Polygon parse_polygon(const string &poly_s)
{
    Polygon ret;
    char br, comma;
    double x, y;
    istringstream iss(poly_s);
    iss >> br; // read [
    while (!iss.eof())
    {
        iss >> br >> x >> comma >> y >> br; // read [1, 2]
        ret.push_back(Point{x, y});
        iss >> comma; // read , or ]

        if (comma == ']')
            break;
    }
    return ret;
}

bool contains(const Polygon &poly, const Point &p)
{
    auto v1 = p - poly[poly.size() - 1];
    auto v2 = poly[0] - poly[poly.size() - 1];

    bool sign = cross_product(v1, v2) > 0;
    for (int i = 1; i < poly.size(); ++i)
    {
        auto v1 = p - poly[i - 1];
        auto v2 = poly[i] - poly[i - 1];
        if (sign != (cross_product(v1, v2) > 0))
            return false;
    }
    return true;
}

bool contains_all(const Polygon &a, const vector<Point> &ps)
{
    return all_of(ps.begin(), ps.end(), [&a](const Point &p) {
        return contains(a, p);
    });
}

bool contains_any(const Polygon &a, const vector<Point> &ps)
{
    return any_of(ps.begin(), ps.end(), [&a](const Point &p) {
        return contains(a, p);
    });
}

bool crosses(const Polygon &a, const Polygon &b)
{
    // TODO check bounds cross
    return false;
}

bool disjoint(const Polygon &a, const Polygon &b)
{
    if (contains_any(a, b))
    {
        return false;
    }
    else if (contains_any(b, a))
    {
        return false;
    }
    else if (crosses(a, b))
    {
        return false;
    }
    return true;
}

bool intersects(const Polygon &a, const Polygon &b)
{
    return !disjoint(a, b);
}

double area(const Polygon &poly)
{
    double ret = 0;
    for (int i = 0; i < poly.size(); ++i)
    {
        ret += cross_product(poly[i], poly[(i + 1) % poly.size()]);
    }
    return 0.5 * ret;
}

string to_string(const Polygon &poly)
{
    string ret = "[";
    for (const auto &p : poly)
    {
        ret += to_string(p) + ", ";
    }
    // ", " -> "]"
    ret.pop_back();
    ret[ret.size() - 1] = ']';
    return ret;
}

tuple<double, double, double, double> Discretizer::axis_aligned_bounding_box_lu_corner(const Polygon &poly,
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

Polygon Discretizer::axis_aligned_bounding_box(const Polygon &poly,
                                               const function<double(double)> &min_trunc,
                                               const function<double(double)> &max_trunc) const
{
    double minx, miny, maxx, maxy;
    tie(minx, miny, maxx, maxy) = axis_aligned_bounding_box_lu_corner(poly, min_trunc, max_trunc);
    return {Point{minx, miny}, Point{maxx, miny}, Point{maxx, maxy}, Point{minx, maxy}};
}

CellSet Discretizer::discretize(const Polygon &poly, bool keep_edge_cells) const
{
    auto get_cell_polygon = [this](int xi, int yi) {
        double x = xi * delta;
        double y = yi * delta;
        Polygon poly;
        for (int i = 0; i < 4; ++i)
        {
            poly.push_back(Point{x + (i % 2) * delta, y + (i / 2) * delta});
        }
        return poly;
    };

    using ConditionType = function<bool(int, int)>;

    ConditionType inside = [&poly, get_cell_polygon](int xi, int yi) {
        auto cell_poly = get_cell_polygon(xi, yi);
        return contains_all(poly, cell_poly);
    };

    ConditionType intersection = [&poly, get_cell_polygon](int xi, int yi) {
        auto cell_poly = get_cell_polygon(xi, yi);
        return intersects(poly, cell_poly);
    };

    ConditionType condition = keep_edge_cells ? intersection : inside;

    double minx, miny, maxx, maxy;
    if (keep_edge_cells)
    {
        tie(minx, miny, maxx, maxy) = axis_aligned_bounding_box_lu_corner(poly, static_cast<double (*)(double)>(floor), static_cast<double (*)(double)>(ceil));
    }
    else
    {
        tie(minx, miny, maxx, maxy) = axis_aligned_bounding_box_lu_corner(poly, static_cast<double (*)(double)>(ceil), static_cast<double (*)(double)>(floor));
    }

    int minxi = round(minx / delta);
    int minyi = round(miny / delta);
    int maxxi = round(maxx / delta);
    int maxyi = round(maxy / delta);

    CellSet ret;
    for (int i = minxi; i < maxxi; ++i)
    {
        for (int j = minyi; j < maxyi; ++j)
        {
            if (condition(i, j))
            {
                ret.insert(index_to_cid(i, j));
            }
        }
    }
    return ret;
}

CID Discretizer::index_to_cid(int xi, int yi) const
{
    return xi + ((CID)yi << 32);
}

Point Discretizer::cid_to_point(const CID &cid) const
{
    int xi = cid & (~0);
    int yi = cid >> 32;
    return {xi * delta, yi * delta};
}

CID Discretizer::point_to_cid(const Point &p) const
{
    int xi = (int)(p.x / delta);
    int yi = (int)(p.y / delta);
    return index_to_cid(xi, yi);
}