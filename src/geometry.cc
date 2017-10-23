#include "geometry.h"

#include <algorithm>
#include <functional>
#include <sstream>
#include <iostream>

using namespace std;

Vector2 Vector2::operator+(const Vector2 &other) const
{
    return Vector2{x + other.x, y + other.y};
}

Vector2 Vector2::operator-(const Vector2 &other) const
{
    return Vector2{x - other.x, y - other.y};
}

bool Vector2::operator==(const Vector2 &other) const
{
    return x == other.x && y == other.y;
}

double cross(const Vector2 &a, const Vector2 &b)
{
    return a.x * b.y - a.y * b.x;
}

string to_string(const Vector2 &v)
{
    return "[" + std::to_string(v.x) + ", " + std::to_string(v.y) + "]";
}

string to_string(const Polygon &poly)
{
    if (poly.size() == 0)
        return "[]";
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

ostream &operator<<(ostream &os, const Vector2 &v)
{
    os << to_string(v);
    return os;
}

ostream &operator<<(ostream &os, const Polygon &poly)
{
    os << to_string(poly);
    return os;
}

Polygon parse_polygon(const string &s)
{
    Polygon ret;
    char br, comma;
    double x, y;
    istringstream iss(s);
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

bool inside(const Point &p, const Point &a, const Point &b) // inside a line means on the left side of on it
{
    return cross(b - a, p - a) > -1e-7;
}

bool inside(const Point &p, const Polygon &poly)
{
    auto s = poly.back();
    for (const auto &e : poly)
    {
        if (!inside(p, s, e))
        {
            return false;
        }
        s = e;
    }
    return true;
}

bool all_inside(const list<Point> &ps, const Polygon &a)
{
    return all_of(ps.begin(), ps.end(), [&a](const Point &p) {
        return inside(p, a);
    });
}

bool any_inside(const list<Point> &ps, const Polygon &a)
{
    return any_of(ps.begin(), ps.end(), [&a](const Point &p) {
        return inside(p, a);
    });
}

bool disjoints(const Polygon &a, const Polygon &b)
{
    return !intersects(a, b);
}

bool intersects(const Polygon &a, const Polygon &b)
{
    return intersection(a, b).size() > 0;
}

bool convex(const Polygon &poly)
{
    auto prev = poly.begin();
    auto cur = next(prev);
    auto post = next(cur);
    while (post != poly.end())
    {
        if (!inside(*post, *prev, *cur))
        {
            return false;
        }
        prev = cur;
        cur = post;
        post++;
    }
    return true;
}

double area(const Polygon &poly)
{
    double ret = 0;
    auto s = poly.back();
    for (const auto &e : poly)
    {
        ret += cross(s, e);
        s = e;
    }
    return 0.5 * ret;
}

list<Triangle> triangulate(const Polygon &poly)
{
    list<Triangle> ret;

    enum Type
    {
        CONVEX,
        EARTIP,
        REFLEX,
    };

    struct Vertex
    {
        Point p;
        Type t;
    };

    list<Vertex> vs;
    for (const auto &p : poly)
    {
        vs.push_back(Vertex{p, CONVEX});
    }
    using Iter = decltype(vs.begin());

    auto get_prev = [&vs](const Iter &it) {
        return it == vs.begin() ? prev(vs.end()) : prev(it);
    };

    auto get_post = [&vs](const Iter &it) {
        return next(it) == vs.end() ? vs.begin() : next(it);
    };

    auto is_reflex = [](const Iter &cur, const Iter &pre, const Iter &pst) {
        return cross(cur->p - pre->p, pst->p - pre->p) < 0;
    };

    auto no_rv_inside = [&vs](const Iter &cur, const Iter &pre, const Iter &pst) {
        return all_of(vs.begin(), vs.end(), [pre, cur, pst](const Vertex &v) {
            return v.t != REFLEX || !inside(v.p, Polygon{pre->p, cur->p, pst->p});
        });
    };

    auto update_reflex = [get_prev, get_post, is_reflex](const Iter &cur) {
        auto pre = get_prev(cur);
        auto pst = get_post(cur);
        if (is_reflex(cur, pre, pst))
        {
            cur->t = REFLEX;
        }
        else if (cur->t == REFLEX)
        {
            cur->t = CONVEX;
        }
    };

    auto update_eartip = [get_prev, get_post, no_rv_inside](const Iter &cur) {
        if (cur->t != REFLEX)
        {
            auto pre = get_prev(cur);
            auto pst = get_post(cur);
            if (no_rv_inside(cur, pre, pst))
            {
                cur->t = EARTIP;
            }
            else // disqualify the eartip
            {
                cur->t = CONVEX;
            }
        }
    };

    auto find_eartip = [&vs]() {
        for (auto it = vs.begin(); it != vs.end(); ++it)
        {
            if (it->t == EARTIP)
            {
                return it;
            }
        }
        return vs.end();
    };

    { // initialize vs
        for (auto it = vs.begin(); it != vs.end(); ++it)
        {
            update_reflex(it);
        }
        for (auto it = vs.begin(); it != vs.end(); ++it)
        {
            update_eartip(it);
        }
    }

    while (vs.size() > 2)
    {
        auto eartip = find_eartip();
        auto pre = get_prev(eartip);
        auto pst = get_post(eartip);
        ret.push_back(Triangle{pre->p, eartip->p, pst->p});
        vs.erase(eartip);
        update_reflex(pre);
        update_reflex(pst);
        update_eartip(pre);
        update_eartip(pst);
    }

    return ret;
}

Point line_line_intersection(const Point &a, const Point &b, const Point &c, const Point &d)
{
    double denominator = cross(a, c) + cross(b, d) + cross(c, b) + cross(d, a);
    if (denominator == 0)
    {
        throw "Error: denominator = 0";
    }
    double numerator_part1 = cross(a, b);
    double numerator_part2 = cross(c, d);
    Point e = {(numerator_part1 * (c.x - d.x) - numerator_part2 * (a.x - b.x)) / denominator,
               (numerator_part1 * (c.y - d.y) - numerator_part2 * (a.y - b.y)) / denominator};
    return e;
}

Polygon intersection(const Polygon &clippee, const Polygon &clipper)
{
    if (!convex(clipper))
    {
        throw "Error: clipper should be convex!";
    }

    auto output_list = clippee;
    auto s1 = clipper.back();
    for (const auto &e1 : clipper)
    {
        auto input_list = output_list;
        output_list.clear();
        auto s2 = input_list.back();
        for (const auto &e2 : input_list)
        {
            if (inside(e2, s1, e1))
            {
                if (!inside(s2, s1, e1))
                {
                    output_list.push_back(line_line_intersection(s2, e2, s1, e1));
                }
                output_list.push_back(e2);
            }
            else if (inside(s2, s1, e1))
            {
                output_list.push_back(line_line_intersection(s2, e2, s1, e1));
            }
            s2 = e2;
        }
        s1 = e1;
    }
    return output_list;
}

list<Polygon> difference(const Polygon &clippee, const Polygon &clipper)
{
    if (!convex(clipper))
    {
        throw "Error: clipper should be convex!";
    }

    list<Polygon> ret;
    auto output_list = clippee;
    auto s1 = clipper.back();
    for (const auto &e1 : clipper)
    {
        auto offcut = list<Point>();
        auto input_list = output_list;
        output_list.clear();
        auto s2 = input_list.back();
        for (const auto &e2 : input_list)
        {
            if (inside(e2, s1, e1))
            {
                if (!inside(s2, s1, e1))
                {
                    auto p = line_line_intersection(s2, e2, s1, e1);
                    output_list.push_back(p);
                    offcut.push_back(s2);
                    offcut.push_back(p);
                }
                output_list.push_back(e2);
            }
            else
            {
                if (inside(s2, s1, e1))
                {
                    auto p = line_line_intersection(s2, e2, s1, e1);
                    output_list.push_back(p);
                    offcut.push_back(p);
                }
                offcut.push_back(e2);
            }
            s2 = e2;
        }
        s1 = e1;
        if (offcut.size() > 0)
        {
            ret.push_back(offcut);
        }
    }
    if (output_list.size() == 0) // no intersection
    {
        ret.clear();
        ret.push_back(clippee);
    }
    return ret;
}