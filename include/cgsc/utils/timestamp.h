#pragma once
#ifndef CGSC_UTILS_TIMESTAMP_H
#define CGSC_UTILS_TIMESTAMP_H

#include <memory>
#include <string>
#include <ctime>

namespace cgsc
{
namespace utils
{

class Timestamp
{
public:
  Timestamp();

  static void Begin(const std::string& tag);
  static double End();

private:
  static clock_t beginTime;

  static std::string currentTag;
};
}
}
#endif
