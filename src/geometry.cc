#include "geometry.h"

#include <algorithm>
#include <queue>
#include <functional>
#include <sstream>
#include <iostream>
#include <cassert>
#include <memory>

using namespace std;

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

enum class Orientation
{
    left = 1,
    right = -1,
    collinear = 0,
};

Orientation calculate_orientation(Point a, Point b, Point c)
{
    auto det = cross(b - a, c - a);
    return static_cast<Orientation>(
        static_cast<int>(strictly_less(0.f, det)) -
        static_cast<int>(strictly_less(det, 0.f)));
}

bool collinear(const Point &p, const Segment &segment)
{
    // return calculate_orientation(p, segment.a, segment.b) == Orientation::collinear;
    auto u = segment.b - segment.a;
    auto v = p - segment.b;
    return abs(cross(u, v) / magnitude(u)) < 1e-5; // i.e. the distance from p to ab / |ab| < threshold
}

bool left(const Point &p, const Segment &segment)
{
    return !collinear(p, segment) &&
           calculate_orientation(segment.a, segment.b, p) == Orientation::left;
}

bool right(const Point &p, const Segment &segment)
{
    return !collinear(p, segment) &&
           calculate_orientation(p, segment.a, segment.b) == Orientation::right;
}

bool inside(const Point &p, const Polygon &polygon)
{
    auto s = polygon.back();
    for (const auto &e : polygon)
    {
        if (!left(p, {s, e}))
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
        if (right(p, {s, e}))
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
    return left(a, {c, d}) == right(b, {c, d}) && left(c, {a, b}) == right(d, {a, b});
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
    auto x_diff = vector2<double>(a.x - b.x, c.x - d.x);
    auto y_diff = vector2<double>(a.y - b.y, c.y - d.y);
    double det = cross(x_diff, y_diff);
    if (det == 0)
    {
        throw runtime_error("Error: divisor = 0 in line_intersection\n" + to_string(Polygon{a, b}) + "," + to_string(Polygon{c, d}));
    }
    auto cr = Point(cross(a, b), cross(c, d));
    auto e = Point(cross(cr, x_diff) / det, cross(cr, y_diff) / det);
    return e;
}

tuple<Polygons, Polygons> clip(const Polygon &clippee, const Polygon &clipper) // inner, outer
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
        return make_tuple(Polygons{}, Polygons{clippee});
    }

    auto inners = Polygons();
    auto outers = Polygons();

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
            if (left(s2, {s1, e1}))
            {
                if (left(e2, {s1, e1}))
                {
                    inner.push_back(e2);
                }
                else if (collinear(e2, {s1, e1}))
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
            else if (collinear(s2, {s1, e1}))
            {
                if (left(e2, {s1, e1}))
                {
                    if (inner.back() != s2) // check bouncing
                    {
                        inner.push_back(s2);
                    }
                    inner.push_back(e2);
                }
                else if (collinear(e2, {s1, e1}))
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
                if (left(e2, {s1, e1}))
                {
                    auto p = line_intersection(s2, e2, s1, e1);
                    inner.push_back(p);
                    inner.push_back(e2);
                    outer.push_back(p);
                }
                else if (collinear(e2, {s1, e1}))
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

Polygons intersection(const Polygon &clippee, const Polygon &clipper)
{
    auto inners = Polygons();
    auto outers = Polygons();
    tie(inners, outers) = clip(clippee, clipper);
    assert(inners.size() < 2); // important
    return inners;
}

Polygons intersection(Polygons clippees, const Polygons &clippers)
{
    Polygons result;
    for (const auto &clipper : clippers)
    {
        for (auto &clippee : clippees)
        {
            result.splice(result.end(), intersection(clippee, clipper));
        }
    }
    return result;
}

Polygons difference(const Polygon &clippee, const Polygon &clipper)
{
    auto inners = Polygons();
    auto outers = Polygons();
    tie(inners, outers) = clip(clippee, clipper);
    return outers;
}

Polygons difference(Polygons clippees, const Polygons &clippers)
{
    for (const auto &clipper : clippers)
    {
        Polygons result;
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
        minx = min(minx, point.x);
        miny = std::min(miny, point.y);
        maxx = std::max(maxx, point.x);
        maxy = std::max(maxy, point.y);
    }
    return box({minx, miny}, {maxx, maxy});
}

#if 0
namespace sweep_line
{
Polygons intersection(const Polygons &polygons)
{
    struct Event
    {
    };

    struct EndPoint : Event, Point
    {
        EndPoint *bottom;
        Polygon *polygon;
    };

    struct IntersectionPoint : Event, Point
    {
    };

    struct Segment
    {
        union {
            EndPoint a, b;
            array<EndPoint, 2> components;
        };
        EndPoint &operator[](int index) { return components[index]; }
        const EndPoint &operator[](int index) const { return components[index]; }
    };

    for (const auto &polygon : polygons)
    {
        const auto &a = polygon.back();
        for (const auto &b : polygon)
        {
            if (a.y > b.y)
            {
            }
            else
            {
            }
            a = b;
        }
    }
}
}

namespace dcel
{
struct Vertex;
struct HalfEdge;
struct Face;

struct Vertex
{
    Point point;
    HalfEdge *edge; // the half edge whose origin is this vertex
};

struct HalfEdge
{
    Vertex *origin;
    HalfEdge *twin;
    HalfEdge *next;
    Face *face;
};

struct Face
{
    vector<int> covers;
};

template <class T>
unique_ptr<T> copy_unique(const unique_ptr<T> &source)
{
    return source ? make_unique<T>(*source) : nullptr;
}

template <class T>
T *get_ptr(const unique_ptr<T> &source)
{
    return source.get();
}

namespace func
{
template <class T,
          template <class = T, class = std::allocator<T>> class Container,
          class Func>
auto map(const Container<T> &iterable, Func &&func)
    -> Container<decltype(func(std::declval<T>())),
                 std::allocator<decltype(func(std::declval<T>()))>>
{
    using value_type = decltype(func(std::declval<T>()));
    using result_type = Container<value_type, std::allocator<value_type>>;

    result_type res;

    for (const auto &v : iterable)
    {
        res.push_back(func(v));
    }
    return res;
}
}

class Mesh
{
  public:
    Mesh() {}
    Mesh(const Polygon &polygon, int id)
    {
        // only 1-face polygon is allowed here
        auto face = append_new_to(faces);
        face->covers.push_back(id);

        HalfEdge *prev_left = nullptr;
        HalfEdge *prev_right = nullptr;

        for (const auto &p : polygon)
        {
            auto vertex = append_new_to(vertices);
            auto left = append_new_to(halfedges);
            auto right = append_new_to(halfedges);

            left->origin = vertex;
            left->next = nullptr;
            left->twin = right;
            left->face = face; // counter-clockwise

            right->origin = nullptr;
            right->next = prev_right;
            right->twin = left;
            right->face = nullptr;

            vertex->edge = left;
            vertex->point = p;

            if (prev_left)
            {
                prev_left->next = left;
            }

            if (prev_right)
            {
                prev_right->origin = vertex;
            }

            prev_left = left;
            prev_right = right;
        }

        prev_right->origin = vertices[0].get();

        auto first_left = halfedges[0].get();
        prev_left->next = first_left;

        auto first_right = halfedges[1].get();
        first_right->next = prev_right;
    }

    vector<Vertex *> get_vertices() const
    {
        return func::map(vertices, get_ptr<Vertex>);
    }

    vector<HalfEdge *> get_halfedges() const
    {
        return func::map(halfedges, get_ptr<HalfEdge>);
    }

    vector<Face *> get_faces() const
    {
        return func::map(faces, get_ptr<Face>);
    }

  private:
    template <class Collection>
    auto append_new_to(Collection &collection) -> typename Collection::value_type::element_type *
    {
        using element_type = typename Collection::value_type::element_type;
        collection.emplace_back(make_unique<element_type>());
        return collection.back().get();
    }

    vector<unique_ptr<Vertex>> vertices;
    vector<unique_ptr<HalfEdge>> halfedges;
    vector<unique_ptr<Face>> faces;
};
}

#endif