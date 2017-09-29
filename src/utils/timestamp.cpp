#include "cgsc/utils/timestamp.h"

#include <ctime>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>

using namespace std;

namespace cgsc
{
namespace utils
{
void Timestamp::Begin(const std::string &tag)
{

    currentTag = tag;
    cout << "\033[1;34m"
         << "["
         << currentTag
         << "]"
         << " ...\033[0m"
         << endl;

    beginTime = clock();
};

double Timestamp::End()
{
    double interval = 0;
    if (currentTag.size() > 0)
    {
        interval = (clock() - beginTime) * 1.0 / CLOCKS_PER_SEC;

        cout << "\033[A\33[2K\r"
             << "\033[1;32m"
             << "["
             << currentTag
             << "]\033[21m"
             << " ends after "
             << "\033[1;32m"
             << fixed
             << setprecision(3)
             << interval
             << "\033[21m"
             << " s"
             << "\033[0m"
             << endl;

        currentTag = "";
    }

    return interval;
};

clock_t Timestamp::beginTime;
string Timestamp::currentTag;
}
}
