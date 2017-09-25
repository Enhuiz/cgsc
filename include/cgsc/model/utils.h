#pragma once
#ifndef CGSC_MODEL_UTILS_HPP
#define CGSC_MODEL_UTILS_HPP

#include <string>
#include <list>
#include <sstream>
#include <cctype>

namespace cgsc
{
namespace model
{

using Point = boost::geometry::model::d2::point_xy<double>;

template <class T>
std::list<T> parseListOf(const std::string &s);

template <>
std::list<double> parseListOf(const std::string &s);

template <>
std::list<Point> parseListOf(const std::string &s);
}
}

#endif