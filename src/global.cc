#include "global.h"

#include <string>
#include <map>
#include <ctime>
#include <iostream>

using namespace std;

Logger logger(cout);
nlohmann::json g_report;
nlohmann::json debug_report;

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

Logger::Buffer::Buffer(ostream &os, const Logger &logger) : output(os), logger(logger)
{
}

int Logger::Buffer::sync()
{
    output << logger.wrap(str());
    str("");
    output.flush();
    return 0;
}

Logger::Logger(ostream &os) : ostream(&buffer), buffer(os, *this)
{
}

void Logger::info(const string &s) const
{
    cout << wrap(s) << endl;
}

void Logger::debug(const string &s) const
{
    cout << "\033[33m" << wrap(s) << "\033[0m" << endl;
}

void Logger::error(const string &s) const
{
    cout << "\033[31m" << wrap(s) << "\033[0m" << endl;
}

string Logger::wrap(const string &s) const
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

string Logger::get_namespaces() const
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