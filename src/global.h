#ifndef CGSC_GLOBAL_H
#define CGSC_GLOBAL_H

#include <limits>
#include <functional>
#include <string>
#include <map>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <memory>
#include <list>

#include "json.hpp"

class Stopwatch
{
public:
  Stopwatch();
  void restart();
  double lap() const;

private:
  clock_t begin_time;
};

class Logger
{
public:
  void info(const std::string &s);
  void debug(const std::string &s);
  void error(const std::string &s);

  void push_namespace(const std::string &ns);
  void pop_namespace();
  std::string get_namespaces();

private:
  std::string wrap(const std::string &s);

private:
  std::list<std::string> nss;
};

class Timer
{
public:
  void begin(const std::string &tag);
  void end();
  void clear();
  void append_to(nlohmann::json& report);

private:
  std::list<std::string> tags;
  std::list<Stopwatch> stopwatches;
  std::map<std::string, std::list<double>> intervals; 
};

extern Logger logger;
extern Timer timer;

template <class T>
typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
almost_equal(T x, T y, int ulp)
{
  // the machine epsilon has to be scaled to the magnitude of the values used
  // and multiplied by the desired precision in ULPs (units in the last place)
  return std::abs(x - y) < std::numeric_limits<T>::epsilon() * std::abs(x + y) * ulp
         // unless the result is subnormal
        //  || std::abs(x - y) < 1e-7; // std::numeric_limits<T>::min();
        || std::abs(x - y) < std::numeric_limits<T>::min();
}

namespace functional
{
template <typename T, typename Func>
auto map(const T &iterable, Func &&func) -> std::vector<decltype(func(std::declval<typename T::value_type>()))>
{
  // Some convenience type definitions
  typedef decltype(func(std::declval<typename T::value_type>())) value_type;
  typedef std::vector<value_type> result_type;

  // Prepares an output vector of the appropriate size
  result_type res(iterable.size());

  // Let std::transform apply `func` to all elements
  // (use perfect forwarding for the function object)
  std::transform(
      begin(iterable), end(iterable), res.begin(),
      std::forward<Func>(func));

  return res;
}
}

template <typename T>
std::string to_string(const T a_value, const int n = 6)
{
  std::ostringstream oss;
  oss << std::setprecision(n) << a_value;
  return oss.str();
}

#endif