#pragma once
#ifndef CGSC_UTILS_TIMESTAMP_H
#define CGSC_UTILS_TIMESTAMP_H

#include <memory>
#include <string>
#include <ctime>

#include "json.hpp"
namespace cgsc
{
namespace utils
{

class Timestamp
{
public:
  Timestamp();

  void add(const std::string &tag);
  void save(const std::string &path) const;
  nlohmann::json toJSON() const;

private:
  double now() const;

private:
  nlohmann::json jobj;
  clock_t startTime;
};
}
}
#endif
