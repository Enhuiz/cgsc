#include "cgsc/utils/timestamp.h"

#include <ctime>
#include <string>
#include <iostream>

using namespace std;

nlohmann::json Timestamp::jobj;
clock_t Timestamp::startTime = clock();

void Timestamp::Add(const string &tag)
{
    jobj[to_string(Now())] = tag;
}

double Timestamp::Now()
{
    return (clock() - startTime) * 1.0 / CLOCKS_PER_SEC;
}

nlohmann::json Timestamp::GetJSON()
{
    return jobj;
}