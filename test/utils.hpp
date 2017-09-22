#ifndef CGSC_TEST_UTILS_HPP
#define CGSC_TEST_UTILS_HPP

#include <list>

namespace cgsc
{
namespace test
{

template <class T>
std::list<T> roll(const std::list<T> &l, int n)
{
    std::list<T> ret = l;
    n = n < 0 ? ret.size() + n : n;

    while (n--)
    {
        ret.push_front(ret.back());
        ret.pop_back();
    }

    return ret;
}

std::string to_string(const std::list<model::Point> &l)
{
    std::string ret;
    std::ostringstream oss(ret);

    oss << "[";
    for (auto i = l.begin(); i != l.end(); ++i)
    {
        const auto &p = *i;
        if (i != l.begin())
        {
            oss << ", ";
        }
        oss << "[" << p.x() << ", " << p.y() << "]";
    }
    oss << "]";

    return oss.str();
}
}
}
#endif