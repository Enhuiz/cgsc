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
    : createdTime(clock())
{
}

void Timestamp::begin(const string &tag)
{
    auto ts = to_string((clock() - createdTime) * 1.0 / CLOCKS_PER_SEC);
    cout << "\033[1;34m"
    << "Timestamp("
    << ts
    << "s): \033[0m"
    << tag << endl;
    
    currentTag = tag;
    beginTime = clock();
}

double Timestamp::end()
{
    if (currentTag.size() > 0)
    {
        auto tmpJobj = nlohmann::json();
        tmpJobj["beginTime"] = (beginTime - createdTime) * 1.0 / CLOCKS_PER_SEC;
        tmpJobj["interval"] = (clock() - beginTime) * 1.0 / CLOCKS_PER_SEC;
        jobj.push_back(tmpJobj);
        currentTag.clear();

        return tmpJobj["interval"];
    }
    return 0;
}

nlohmann::json Timestamp::toJSON() const
{
    return jobj;
}
}
}
