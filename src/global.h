#ifndef CGSC_GLOBAL_H
#define CGSC_GLOBAL_H

#include <limits>
#include <type_traits>
#include <functional>
#include <string>
#include <map>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <memory>
#include <list>

#include "json.hpp"

class Stopwatch
{
public:
  Stopwatch();
  void restart();
  double lap() const;
  void pause();
  void continue_();

private:
  clock_t begin_time;
  double accumulation;
};

namespace func
{
template <class T,
          template <class = T, class = std::allocator<T>> class Container,
          class Func>
auto map(const Container<T> &iterable, Func &&func)
    -> Container<decltype(func(std::declval<T>())),
                 std::allocator<decltype(func(std::declval<T>()))>>
{
  // Some convenience type definitions
  using value_type = decltype(func(std::declval<T>()));
  using result_type = Container<value_type, std::allocator<value_type>>;

  // Prepares an output vector of the appropriate size
  result_type res;

  for (const auto &v : iterable)
  {
    res.push_back(func(v));
  }
  return res;
}

template <typename T, typename Func>
auto filter(const T &iterable, Func &&func) -> T
{
  T ret;
  for (const auto &v : iterable)
  {
    if (func(v))
    {
      ret.push_back(v);
    }
  }
  return ret;
}

template <typename T>
inline auto identity(T v) -> T
{
  return v;
}

template <typename T, typename Func>
auto sum(const T &iterable, Func &&func = identity) -> double
{
  double ret = 0;
  for (const auto &v : iterable)
  {
    ret += func(v);
  }
  return ret;
}

template <typename T, typename Func>
auto min_element(const T &iterable, Func &&func = identity) -> decltype(iterable.begin())
{
  using value_type = typename T::value_type;
  return std::min_element(iterable.begin(),
                          iterable.end(),
                          [&func](const value_type &a, const value_type &b) {
                            return func(a) < func(b);
                          });
}

template <typename T, typename Func>
auto max_element(const T &iterable, Func &&func = identity) -> decltype(iterable.begin())
{
  using value_type = typename T::value_type;
  return std::max_element(iterable.begin(),
                          iterable.end(),
                          [&func](const value_type &a, const value_type &b) {
                            return func(a) < func(b);
                          });
}

template <typename T, typename Func>
auto min(const T &iterable, Func &&func = identity) -> decltype(func(std::declval<typename T::value_type>()))
{
  using result_type = decltype(func(std::declval<typename T::value_type>()));
  if (iterable.size() == 0)
  {
    std::cerr << "min for nothing!" << std::endl;
    std::abort();
  }
  result_type ret = func(*iterable.begin());
  for (auto x : iterable)
  {
    auto v = func(x);
    if (v < ret)
    {
      ret = v;
    }
  }
  return ret;
}

template <typename T, typename Func>
auto max(const T &iterable, Func &&func = identity) -> decltype(func(std::declval<typename T::value_type>()))
{
  using result_type = decltype(func(std::declval<typename T::value_type>()));
  if (iterable.size() == 0)
  {
    std::cerr << "max for nothing!" << std::endl;
    std::abort();
  }
  result_type ret = func(*iterable.begin());
  for (auto x : iterable)
  {
    auto v = func(x);
    if (v < ret)
    {
      ret = v;
    }
  }
  return ret;
}

template <typename T, typename Func>
auto mean(const T &iterable, Func &&func = identity) -> double
{
  double ret = 0;
  for (const auto &v : iterable)
  {
    ret += func(v);
  }
  return ret / iterable.size();
}
}

template <typename T>
std::string to_string(const T &a_value, int n)
{
  std::ostringstream oss;
  oss << std::setprecision(n) << a_value;
  return oss.str();
}

template <class Func>
auto timeit(const std::string &tag, const Func &func) -> decltype(func())
{
  Stopwatch sw;
  auto ret = func();
  std::cout << "timeit " << tag << ": " << sw.lap() << " s" << std::endl;
  return ret;
}

std::list<std::string> split(std::string s, const std::string &delimiter);
std::string now_str();
std::string exec(const std::string &cmd);
void append_polygon_to_online_plotter(const std::string &s);
void clear_online_plotter();
void sleep(int ms);
#endif