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

class Logger: public std::ostream
{
class Buffer : public std::stringbuf
{
  std::ostream& output; 
  const Logger& logger;
public:
  Buffer(std::ostream& os, const Logger& logger);
  virtual int sync();
};

public:
  Logger(std::ostream& os);

  void info(const std::string &s) const;
  void debug(const std::string &s) const;
  void error(const std::string &s) const;

  void push_namespace(const std::string &ns);
  void pop_namespace();
  std::string get_namespaces() const;

private:
  std::string wrap(const std::string &s) const;

private:
  std::list<std::string> nss;
  std::ostringstream oss;
  Buffer buffer;
};

extern Logger logger;
extern nlohmann::json g_report;
extern nlohmann::json debug_report;

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

template <template <class, class> class Container>
double sum(const Container<double, std::allocator<double>> &vs)
{
  double ret = 0;
  for (auto v : vs)
  {
    ret += v;
  }
  return ret;
}

template <template <class, class> class Container>
double mean(const Container<double, std::allocator<double>> &vs)
{
  return sum(vs) / vs.size();
}

template <typename T>
std::string to_string(const T a_value, const int n = 6)
{
  std::ostringstream oss;
  oss << std::setprecision(n) << a_value;
  return oss.str();
}

#endif