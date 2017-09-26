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
  static nlohmann::json GetJSON();
  static void Add(const std::string &tag);
  static void Save(const std::string &path);
private:
  Timestamp();

  static double Now();

private:
  static nlohmann::json jobj;
  static clock_t startTime;
};
}
}
#endif
