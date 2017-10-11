#include <boost/program_options.hpp>

#include "experiment.h"

#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char *argv[])
{
    using namespace boost::program_options;

    auto desc = options_description("Options");
    desc.add_options()("aoi-path,a", value<string>(), "source file of aoi")("scenes-path,s", value<string>(), "source file of scenes")("output-path,o", value<string>(), "output path")("delta,d", value<double>(), "grid length");

    variables_map vm;
    try
    {
        store(parse_command_line(argc, argv, desc), vm);
        notify(vm);
    }
    catch (...)
    {
        cout << desc << endl;
        return 0;
    }

    if (vm.count("delta") && vm.count("aoi-path") && vm.count("scenes-path") && vm.count("output-path"))
    {
        auto report = experiment(vm["aoi-path"].as<string>(), vm["scenes-path"].as<string>(), vm["delta"].as<double>());
        ofstream ofs(vm["output-path"].as<string>());
        ofs << report << endl;
    }
    else
    {
        cout << desc << endl;
    }

    return 0;
}
