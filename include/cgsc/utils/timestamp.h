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

  void begin(const std::string &tag);
  double end();

  nlohmann::json toJSON() const;

private:
  nlohmann::json jobj;

  clock_t createdTime;
  clock_t beginTime;

  std::string currentTag;
};
}
}
#endif
