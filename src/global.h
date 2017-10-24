#ifndef CGSC_GLOBAL_H
#define CGSC_GLOBAL_H

#include <limits>
#include <string>
#include <map>
#include <ctime>
#include <memory>
#include <sstream>
#include <iomanip>

#include "json.hpp"

struct Timer
{
    clock_t begin_time;
    std::string ns; // namespace
    std::string info;
    void begin(const std::string &tag);
    double end();
};

static Timer timer;

template <class T>
typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
almost_equal(T x, T y, int ulp)
{
    // the machine epsilon has to be scaled to the magnitude of the values used
    // and multiplied by the desired precision in ULPs (units in the last place)
    return std::abs(x - y) < std::numeric_limits<T>::epsilon() * std::abs(x + y) * ulp
           // unless the result is subnormal
           || std::abs(x - y) < 1e-7; // std::numeric_limits<T>::min();
}

template <typename T>
std::string to_string(const T a_value, const int n = 6)
{
    std::ostringstream out;
    out << std::setprecision(n) << a_value;
    return out.str();
}

#endif