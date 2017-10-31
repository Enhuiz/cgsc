#include "geometry.h"

#include <algorithm>
#include <functional>
#include <sstream>
#include <iostream>

#include "global.h"

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

bool Vector2::operator!=(const Vector2 &other) const
{
    return x != other.x || y != other.y;
}

bool Vector2::almost_equal(const Vector2 &other, int ulp) const
{
    return ::almost_equal(x, other.x, ulp) && ::almost_equal(y, other.y, ulp);
}

double cross(const Vector2 &a, const Vector2 &b)
{
    return a.x * b.y - a.y * b.x;
}

double dot(const Vector2 &a, const Vector2 &b)
{
    return a.x * b.x + a.y * b.y;
}

double sqr_magnitude(const Vector2 &a)
{
    return dot(a, a);
}

double magnitude(const Vector2 &a)
{
    return sqrt(sqr_magnitude(a));
}

double distance(const Point &a, const Point &b)
{
    return magnitude(a - b);
}

double angle(const Point &a, const Point &b, const Point &c)
{
    auto u = a - b;
    auto v = c - b;
    auto x = dot(u, v) / magnitude(u) / magnitude(v);
    x = max(min(x, 1.0), -1.0); // for precision's sake
    return acos(x) / acos(-1) * 180;
}

string to_string(const Vector2 &v)
{
    return "[" + to_string(v.x, 20) + ", " + to_string(v.y, 20) + "]";
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

bool onside(const Point &p, const Point &a, const Point &b)
{
    auto u = b - a;
    auto v = p - a;
    return abs(cross(u, v)) < 1e-9;
    // return almost_equal(u.x * v.y, u.y * v.x, 1e3);
}

bool inside(const Point &p, const Point &a, const Point &b) // inside a line means on the left side of on it
{
    return cross(b - a, p - a) > 0 && !onside(p, a, b);
}

bool outside(const Point &p, const Point &a, const Point &b)
{
    return cross(b - a, p - a) < 0 && !onside(p, a, b);
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

bool outside(const Point &p, const Polygon &poly)
{
    auto s = poly.back();
    for (const auto &e : poly)
    {
        if (outside(p, s, e))
        {
            return true;
        }
        s = e;
    }
    return false;
}

bool disjoints(const Polygon &a, const Polygon &b)
{
    return !intersects(a, b);
}

bool intersects(const Polygon &a, const Polygon &b)
{
    return intersection(a, b).size() > 0;
}

bool intersects(const Point &a, const Point &b, const Point &c, const Point &d)
{
    return inside(a, c, d) == outside(b, c, d) && inside(c, a, b) == outside(d, a, b);
}

bool simple(const Polygon &poly)
{
    using Iter = decltype(poly.begin());

    auto post = [&poly](const Iter &it) {
        return next(it) == poly.end() ? poly.begin() : next(it);
    };

    for (auto i = poly.begin(); next(next(i)) != poly.end(); ++i)
    {
        auto ip = post(i);
        for (auto j = post(ip); j != poly.end(); ++j)
        {
            auto jp = post(j);
            if (jp != i && intersects(*i, *ip, *j, *jp))
            {
                return false;
            }
        }
    }
    return true;
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
            logger.debug(to_string(angle(*prev, *cur, *post)));
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
        for (const auto &p : poly)
        {
            vs.push_back(Vertex{p, CONVEX});
        }
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
        if (eartip == vs.end())
        {
            list<Point> ps;
            transform(vs.begin(), vs.end(), back_inserter(ps), [](const Vertex &v) {
                return v.p;
            });
            ret.push_back(ps);
            throw runtime_error("Error: eartip is not enough!\n" + to_string(poly) + '\n' + to_string(ps));
        }
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
    auto x_diff = Vector2{a.x - b.x, c.x - d.x};
    auto y_diff = Vector2{a.y - b.y, c.y - d.y};
    double det = cross(x_diff, y_diff);
    if (almost_equal(det, 0.0, 1e3))
    {
        throw runtime_error("Error: divisor = 0 in line_line_intersection\n" + to_string(Polygon{a, b}) + "," + to_string(Polygon{c, d}));
    }
    auto cr = Point{cross(a, b), cross(c, d)};
    auto e = Point{cross(cr, x_diff) / det, cross(cr, y_diff) / det};
    return e;
}

Polygon intersection(const Polygon &clippee, const Polygon &clipper)
{
    if (!convex(clipper))
    {
        throw runtime_error("Error: clipper is non-convex in intersection!\n" + to_string(clipper));
    }

    auto output_list = clippee;
    auto s2 = clipper.back();
    for (const auto &e2 : clipper)
    {
        auto input_list = output_list;
        output_list.clear();
        auto s1 = input_list.back();
        for (const auto &e1 : input_list)
        {
            if (inside(s1, s2, e2))
            {
                if (inside(e1, s2, e2))
                {
                    output_list.push_back(e1);
                }
                else if (onside(e1, s2, e2))
                {
                    output_list.push_back(e1);
                }
                else // outside(e1)
                {
                    auto p = line_line_intersection(s1, e1, s2, e2);
                    output_list.push_back(p);
                }
            }
            else if (onside(s1, s2, e2))
            {
                if (inside(e1, s2, e2))
                {
                    if (output_list.back() != s1) // check bouncing
                    {
                        output_list.push_back(s1);
                    }
                    output_list.push_back(e1);
                }
            }
            else // outside(s1)
            {
                if (inside(e1, s2, e2))
                {
                    auto p = line_line_intersection(s1, e1, s2, e2);
                    output_list.push_back(p);
                    output_list.push_back(e1);
                }
            }
            s1 = e1;
        }
        s2 = e2;
    }
    return output_list;
}

list<Polygon> difference(const Polygon &clippee, const Polygon &clipper)
{
    if (!convex(clipper))
    {
        throw runtime_error("Error: clipper is non-convex in difference!\n" + to_string(clipper));
    }

    list<Polygon> ret;
    auto output_list = clippee;
    auto s2 = clipper.back();
    for (const auto &e2 : clipper)
    {
        auto input_list = output_list;
        output_list.clear();
        auto offcut = list<Point>();
        auto s1 = input_list.back();
        // cout << endl;
        for (const auto &e1 : input_list)
        {
            // if (inside(s1, s2, e2)) {
            //     cout << "inside" << endl;
            // } else if (onside(s1, s2, e2))
            // {
            //     cout << "onside" << endl;
            // } else {
            //     cout << "outside" << endl;
            // }
            // if (inside(e1, s2, e2)) {
            //     cout << "inside" << endl;
            // } else if (onside(s1, s2, e2))
            // {
            //     cout << "onside" << endl;
            // } else {
            //     cout << "outside" << endl;
            // }

            if (inside(s1, s2, e2))
            {
                if (inside(e1, s2, e2))
                {
                    output_list.push_back(e1);
                }
                else if (onside(e1, s2, e2))
                {
                    output_list.push_back(e1);
                }
                else // outside(e1)
                {
                    auto p = line_line_intersection(s1, e1, s2, e2);
                    output_list.push_back(p);
                    offcut.push_back(p);
                    offcut.push_back(e1);
                }
            }
            else if (onside(s1, s2, e2))
            {
                if (inside(e1, s2, e2))
                {
                    if (output_list.back() != s1) // check bouncing
                    {
                        output_list.push_back(s1);
                    }
                    output_list.push_back(e1);
                }
                else if (onside(e1, s2, e2))
                {
                    // do nothing
                }
                else // outside(e1)
                {
                    if (offcut.back() != s1) // check bouncing
                    {
                        offcut.push_back(s1);
                    }
                    offcut.push_back(e1);
                }
            }
            else // outside(s1)
            {
                if (inside(e1, s2, e2))
                {
                    auto p = line_line_intersection(s1, e1, s2, e2);
                    output_list.push_back(p);
                    output_list.push_back(e1);
                    offcut.push_back(p);
                }
                else if (onside(e1, s2, e2))
                {
                    offcut.push_back(e1);
                }
                else // outside(e1)
                {
                    offcut.push_back(e1);
                }
            }
            s1 = e1;
        }
        if (offcut.size() > 0)
        {
            ret.push_back(offcut);
        }
        s2 = e2;
    }
    if (output_list.size() == 0) // no intersection
    {
        ret.clear();
        ret.push_back(clippee);
    }
    return ret;
}