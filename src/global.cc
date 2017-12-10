#include "global.h"

#include <string>
#include <map>
#include <ctime>
#include <iostream>
#include <chrono>

using namespace std;

Stopwatch::Stopwatch()
{
    restart();
}

void Stopwatch::restart()
{
    accumulation = 0;
    begin_time = clock();
}

void Stopwatch::pause()
{
    auto now = clock();
    accumulation += (now - begin_time) * 1.0 / CLOCKS_PER_SEC;
    begin_time = now; // make lap still work after pause before continue
}

void Stopwatch::continue_()
{
    begin_time = clock();
}

double Stopwatch::lap() const
{
    return accumulation + (clock() - begin_time) * 1.0 / CLOCKS_PER_SEC;
}

list<string> split(string s, const string &delimiter)
{
    auto ret = list<string>();
    int pos = 0;
    while ((pos = s.find(delimiter)) != string::npos)
    {
        ret.push_back(s.substr(0, pos));
        s.erase(0, pos + delimiter.size());
    }
    ret.push_back(s);
    return ret;
}

string now_str()
{
    using namespace std::chrono;
    auto now = system_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    auto timer = system_clock::to_time_t(now);
    std::tm bt = *std::localtime(&timer);

    std::ostringstream oss;

    oss << std::put_time(&bt, "%F %T"); // HH:MM:SS
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();

    return oss.str();
}