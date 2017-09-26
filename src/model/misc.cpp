#include <string>
#include <list>
#include <sstream>
#include <cctype>
#include <cassert>

#include <boost/geometry/geometries/point_xy.hpp>

#include "cgsc/model/misc.h"

using namespace std;

namespace cgsc
{
namespace model
{

template <class T>
list<T> parseListOf(const string &s)
{
    return list<T>();
}

template <>
list<double> parseListOf(const string &s)
{
    list<double> ret;
    // required string format:
    // [[0.105, 0.105], [0.2, 0.2], ..., [0.2, 0.2]]

    const auto nexti = [&](const string &s, int &i) {
        while (i < s.size() && !isdigit(s[i]) && s[i] != '.' && s[i] != '-')
            ++i;
    };

    const auto nextdouble = [](const string &s, int &i) {
        unsigned long long digits = 0;
        double decimal = 1;

        bool dot = false;
        int sign = 1;

        if (s[i] == '-')
        {
            sign = -1;
            ++i;
        }

        while (i < s.size())
        {
            if (s[i] == '.')
            {
                dot = true;
            }
            else if (isdigit(s[i]))
            {
                digits *= 10;
                digits += s[i] - '0';
                if (dot)
                {
                    decimal *= 10;
                }
            }
            else
            {
                ++i; // to avoid an extra check
                break;
            }
            ++i;
        }
        return digits / decimal * sign;
    };

    int i = 0;
    nexti(s, i);

    while (i < s.size())
    {
        ret.push_back(nextdouble(s, i));
        nexti(s, i);
    }

    return ret;
}

template <>
list<Point> parseListOf(const string &s)
{
    auto nums = parseListOf<double>(s);

    list<Point> points;
    for (auto i = nums.begin(); i != nums.end(); ++i)
    {
        double x = *i;
        double y = *(++i);
        points.emplace_back(x, y);
    }

    return points;
}
}
}
