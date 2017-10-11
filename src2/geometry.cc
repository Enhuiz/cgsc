#include "geometry.h"

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

double : cross_product(const Vector2 &a, const Vector2 &b)
{
    return a.x * b.y - a.y * b.x;
}

string to_string(const Vector2 &v2)
{
    return "[" + ::to_string(v2.x) + ", " + ::to_string(v2.y) + "]";
}

bool contains(const Polygon &poly, const Point &p)
{
    auto v1 = p - poly[poly.size() - 1];
    auto v2 = poly[0] - poly[poly.size() - 1];
    bool sign = v1.cross_product(v2) > 0;
    for (int i = 1; i < poly.size(); ++i)
    {
        auto v1 = p - poly[i - 1];
        auto v2 = poly[i] - poly[i - 1];
        if (sign != (v1.cross_product(v2) > 0))
            return false;
    }
    return true;
}

bool contains(const Polygon &a, const Polygon &b)
{
    return all_of(b.begin(), b.end(), [](const Point &p) {
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
    if (contains(a, b))
    {
        return false;
    }
    else if (contains(b, a))
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

double area(const Polygon &poly) const
{
    double ret = 0;
    for (int i = 0; i < poly.size(); ++i)
    {
        auto v1 = poly[i] - p;
        auto v2 = poly[(i + 1) % poly.size()] - p;
        ret += v1.cross_product(v2);
    }
    return 0.5 * ret;
}

string to_string(const Polygon &poly) const
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

CellSet Discretizer::discretize(const Polygon &poly, bool keep_edge_point)
{
    auto get_cell_polygon = [this->delta](int xi, int yi)
    {
        Polygon poly;
        for (int i = 0; i < 4; ++i)
        {
            poly.emplace_back({x + (i % 2) * delta, y + (i / 2) * delta});
        }
        return poly;
    };

    auto inside = [&poly, get_cell_polygon](int xi, int yi) {
        auto cell_poly = get_cell_polygon(xi, yi);
        return contains(poly, cell_poly);
    };

    auto intersection = [&poly, get_cell_polygon](int xi, int yi) {
        auto cell_poly = get_cell_polygon(xi, yi);
        return intersects(poly, cell_poly);
    };

    auto min_trunc = keep_edge_point ? static_cast<double (*)(double)> floor : static_cast<double (*)(double)> ceil;
    auto max_trunc = keep_edge_point ? static_cast<double (*)(double)> ceil : static_cast<double (*)(double)> floor;
    auto condition = keep_edge_point ? intersection : inside;

    Point min_p, max_p;
    min_p = max_p = poly[0];
    for (const auto &p : poly)
    {
        min_p.x = min(min_p.x, p.x);
        min_p.y = min(min_p.y, p.y);
        max_p.x = max(max_p.x, p.x);
        max_p.y = max(max_p.y, p.y);
    }

    min_p.x = min_trunc(min_p.x);
    min_p.y = min_trunc(min_p.y);
    max_p.x = max_trunc(max_p.x);
    max_p.y = max_trunc(max_p.y);

    CellSet ret;
    for (int i = min_p.x; i < max_p.x; ++i)
    {
        for (int j = min_p.y; j < max_p.y; ++j)
        {
            if (condition(i, j))
            {
                ret.insert(index_to_cid(i, j));
            }
        }
    }
    return ret;
}

CellID Discretizer::index_to_cid(int xi, int yi)
{
    return xi + ((CID)yi << 32);
}

Point Discretizer::cid_to_point(const CID &cid)
{
    int xi = cid & (~0);
    int yi = cid >> 32;
    return {xi * delta, yi * delta};
}

CID Discretizer::point_to_cid(const Point &p)
{
    int xi = (int)(p.x / delta);
    int yi = (int)(p.y / delta);
    return index_to_cid(xi, yi);
}