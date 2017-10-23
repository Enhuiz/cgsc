#include "global.h"

#include <string>
#include <map>
#include <ctime>
#include <iostream>

using namespace std;

void Timer::begin(const string &tag)
{
    this->info = ns + "::" + tag;
    cout << "\033[1;34m"
         << "["
         << info
         << "]"
         << " ...\033[0m"
         << endl;

    begin_time = clock();
}

double Timer::end()
{
    double interval = (clock() - begin_time) * 1.0 / CLOCKS_PER_SEC;
    cout << "\033[A\33[2K\r"
         << "\033[1;32m"
         << "["
         << info
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
    info = "";
    return interval;
}

