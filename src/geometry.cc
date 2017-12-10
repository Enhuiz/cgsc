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

bool Vector2::operator!=(const Vector2 &other) const
{
    return x != other.x || y != other.y;
}

Vector2 Vector2::operator*(double a) const
{
    return Vector2{x * a, y * a};
}

Vector2 Vector2::operator/(double a) const
{
    return Vector2{x / a, y / a};
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
    return "[" + to_string(v.x) + ", " + to_string(v.y) + "]";
}

string to_string(const Polygon &polygon)
{
    if (polygon.size() == 0)
        return "[]";
    string ret = "[";
    for (const auto &p : polygon)
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

ostream &operator<<(ostream &os, const Polygon &polygon)
{
    os << to_string(polygon);
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
    if (ret.back() == ret.front())
    { // remove duplicated
        ret.pop_back();
    }
    return ret;
}

bool onside(const Point &p, const Point &a, const Point &b)
{
    auto u = b - a;
    auto v = p - a;
    return abs(cross(u, v) / magnitude(u)) < 1e-5; // i.e. the distance from p to ab / |ab| < threshold
}

bool inside(const Point &p, const Point &a, const Point &b) // inside a line means on the left side of on it
{
    return cross(b - a, p - a) > 0 && !onside(p, a, b);
}

bool outside(const Point &p, const Point &a, const Point &b)
{
    return cross(b - a, p - a) < 0 && !onside(p, a, b);
}

bool inside(const Point &p, const Polygon &polygon)
{
    auto s = polygon.back();
    for (const auto &e : polygon)
    {
        if (!inside(p, s, e))
        {
            return false;
        }
        s = e;
    }
    return true;
}

bool outside(const Point &p, const Polygon &polygon)
{
    auto s = polygon.back();
    for (const auto &e : polygon)
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

bool convex(const Polygon &polygon)
{
    auto a = *prev(prev(polygon.end()));
    auto b = *prev(polygon.end());
    for (const auto &c : polygon)
    {
        if (area(Polygon{a, b, c}) < 0)
        {
            return false;
        }
        a = b;
        b = c;
    }
    return true;
}

double area(const Polygon &polygon)
{
    double ret = 0;
    auto s = polygon.back();
    for (const auto &e : polygon)
    {
        ret += cross(s, e); // os x oe, o is the origin point (0, 0).
        s = e;
    }
    return 0.5 * ret;
}

Point line_intersection(const Point &a, const Point &b, const Point &c, const Point &d)
{
    auto x_diff = Vector2{a.x - b.x, c.x - d.x};
    auto y_diff = Vector2{a.y - b.y, c.y - d.y};
    double det = cross(x_diff, y_diff);
    if (det == 0)
    {
        throw runtime_error("Error: divisor = 0 in line_intersection\n" + to_string(Polygon{a, b}) + "," + to_string(Polygon{c, d}));
    }
    auto cr = Point{cross(a, b), cross(c, d)};
    auto e = Point{cross(cr, x_diff) / det, cross(cr, y_diff) / det};
    return e;
}

tuple<list<Polygon>, list<Polygon>> clip(const Polygon &clippee, const Polygon &clipper) // inner, outer
{
    double clippee_area = area(clippee);
    double clipper_area = area(clipper);
    double threshold_area = max(clippee_area, clipper_area) * 1e-6; // if a piece is smaller than this, discard it.
    if (!convex(clipper))
    {
        cerr << "Clip Error: clipper is non-convex!" << endl
             << "polygon: " << to_string(clipper) << endl
             << "clippee area: " << clippee_area << endl
             << "clipper area: " << clipper_area << endl;
        return make_tuple(list<Polygon>{}, list<Polygon>{clippee});
    }

    auto inners = list<Polygon>();
    auto outers = list<Polygon>();

    auto inner = clippee;
    auto s1 = clipper.back();
    for (const auto &e1 : clipper)
    {
        auto prev_inner = move(inner);
        inner.clear();
        auto outer = list<Point>();
        auto s2 = prev_inner.back();
        for (const auto &e2 : prev_inner)
        {
            if (inside(s2, s1, e1))
            {
                if (inside(e2, s1, e1))
                {
                    inner.push_back(e2);
                }
                else if (onside(e2, s1, e1))
                {
                    inner.push_back(e2);
                }
                else // outside(e2)
                {
                    auto p = line_intersection(s2, e2, s1, e1);
                    inner.push_back(p);
                    outer.push_back(p);
                    outer.push_back(e2);
                }
            }
            else if (onside(s2, s1, e1))
            {
                if (inside(e2, s1, e1))
                {
                    if (inner.back() != s2) // check bouncing
                    {
                        inner.push_back(s2);
                    }
                    inner.push_back(e2);
                }
                else if (onside(e2, s1, e1))
                {
                    // do nothing
                }
                else // outside(e2)
                {
                    if (outer.back() != s2) // check bouncing
                    {
                        outer.push_back(s2);
                    }
                    outer.push_back(e2);
                }
            }
            else // outside(s2)
            {
                if (inside(e2, s1, e1))
                {
                    auto p = line_intersection(s2, e2, s1, e1);
                    inner.push_back(p);
                    inner.push_back(e2);
                    outer.push_back(p);
                }
                else if (onside(e2, s1, e1))
                {
                    outer.push_back(e2);
                }
                else // outside(e2)
                {
                    outer.push_back(e2);
                }
            }
            s2 = e2;
        }
        if (area(outer) > threshold_area)
        {
            outers.push_back(outer);
        }
        s1 = e1;
    }

    if (area(inner) > threshold_area)
    {
        inners.push_back(inner);
    }
    else // no intersection, restore the cut clippee
    {
        outers.clear();
        outers.push_back(clippee);
    }

    for (const auto &outer : outers)
    {
        if (!convex(outer))
        {
            cerr << "nonconvex found!" << endl;
            cerr << "a = " << to_string(clippee) << endl;
            cerr << "b = " << to_string(clipper) << endl;
            cerr << "c = " << to_string(outer) << endl;
            abort();
        }
    }

    return make_tuple(inners, outers);
}

list<Polygon> intersection(const Polygon &clippee, const Polygon &clipper)
{
    auto inners = list<Polygon>();
    auto outers = list<Polygon>();
    tie(inners, outers) = clip(clippee, clipper);
    return inners;
}

list<Polygon> intersection(list<Polygon> clippees, const list<Polygon> &clippers)
{
    list<Polygon> result;
    for (const auto &clipper : clippers)
    {
        for (auto &clippee : clippees)
        {
            result.splice(result.end(), intersection(clippee, clipper));
        }
    }
    return result;
}

list<Polygon> difference(const Polygon &clippee, const Polygon &clipper)
{
    auto inners = list<Polygon>();
    auto outers = list<Polygon>();
    tie(inners, outers) = clip(clippee, clipper);
    return outers;
}

list<Polygon> difference(list<Polygon> clippees, const list<Polygon> &clippers)
{
    for (const auto &clipper : clippers)
    {
        list<Polygon> result;
        for (auto &clippee : clippees)
        {
            result.splice(result.end(), difference(clippee, clipper));
        }
        clippees = move(result);
    }
    return clippees;
}

Polygon box(const Point &lower_left, const Point &upper_right)
{
    return Polygon{lower_left, {upper_right.x, lower_left.y}, upper_right, {lower_left.x, upper_right.y}};
}

Polygon axis_aligned_bounding_box(const Polygon &polygon)
{
    double minx, miny, maxx, maxy;
    minx = maxx = polygon.begin()->x;
    miny = maxy = polygon.begin()->y;
    for (const auto &point : polygon)
    {
        minx = std::min(minx, point.x);
        miny = std::min(miny, point.y);
        maxx = std::max(maxx, point.x);
        maxy = std::max(maxy, point.y);
    }
    return box({minx, miny}, {maxx, maxy});
}