#include "global.h"

#include <string>
#include <map>
#include <ctime>
#include <iostream>

using namespace std;

Logger logger;
Timer timer;

Stopwatch::Stopwatch()
{
    restart();
}

void Stopwatch::restart()
{
    begin_time = clock();
}

double Stopwatch::lap() const
{
    return (clock() - begin_time) * 1.0 / CLOCKS_PER_SEC;
}

void Logger::info(const string &s)
{
    cout << wrap(s) << endl;
}

void Logger::debug(const string &s)
{
    cout << "\033[33m" << wrap(s) << "\033[0m" << endl;
}

string Logger::wrap(const string &s)
{
    return "[" + get_namespaces() + "] " + s;
}

void Logger::push_namespace(const string &ns)
{
    nss.push_back(ns);
}

void Logger::pop_namespace()
{
    nss.pop_back();
}

string Logger::get_namespaces()
{
    string ret;
    for (auto it = nss.begin(); it != nss.end(); ++it)
    {
        if (it == nss.begin())
        {
            ret += *it;
        }
        else
        {
            ret += "::" + *it;
        }
    }
    return ret;
}

void Timer::begin(const string &event)
{
    this->event = event;
    logger.info(event + " ...");
    stopwatch.restart();
}

double Timer::end()
{
    double interval = stopwatch.lap();
    logger.info(event + " ends after " + to_string(interval) + " s");
    event = "";
    return interval;
}
