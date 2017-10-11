#ifndef CGSC_GLOBAL_H
#define CGSC_GLOBAL_H

#include <string>
#include <map>
#include <ctime>
#include <memory>

#include "json.hpp"

struct Timer
{
    clock_t begin_time;
    std::string tag;
    void begin(const std::string& tag);
    double end();
};

static Timer timer;

#endif