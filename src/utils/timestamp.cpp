#include "cgsc/utils/timestamp.h"

#include <ctime>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

namespace cgsc
{
namespace utils
{
Timestamp::Timestamp()
    : startTime(clock())
{
}

void Timestamp::add(const string &tag)
{
    auto ts = to_string(now());
    jobj[ts] = tag;
    cout << "\033[1;34m"
         << "Timestamp("
         << ts
         << "s): \033[0m"
         << tag << endl;
}

double Timestamp::now() const
{
    return (clock() - startTime) * 1.0 / CLOCKS_PER_SEC;
}

nlohmann::json Timestamp::toJSON() const
{
    return jobj;
}

void Timestamp::save(const std::string &path) const
{
    ofstream ofs(path);
    ofs << jobj << endl;
}
}
}
