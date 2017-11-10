#include <boost/program_options.hpp>

#include "experiment.h"
#include "global.h"

#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char *argv[])
{
    using namespace boost::program_options;

    auto desc = options_description("Options");
    desc.add_options()("roi-path,r", value<string>(), "source file of roi")("scenes-path,s", value<string>(), "source file of scenes")("output-path,o", value<string>(), "output path")("delta,d", value<double>(), "grid length")("target-coverage,t", value<double>(), "target coverage");

    variables_map vm;
    try
    {
        store(parse_command_line(argc, argv, desc), vm);
        notify(vm);
    }
    catch (...)
    {
        cout << "Error: parse error" << endl;
        cout << desc << endl;
        return 0;
    }

    if (vm.count("delta") && vm.count("roi-path") && vm.count("scenes-path") && vm.count("output-path") && vm.count("target-coverage"))
    {
        auto report = experiment(vm["roi-path"].as<string>(), vm["scenes-path"].as<string>(), vm["target-coverage"].as<double>(), vm["delta"].as<double>());
        {
            ofstream ofs(vm["output-path"].as<string>());
            ofs << report << endl;
        }
        {
            ofstream ofs("../data/visualize/bnb/vinfo.json");
            ofs << debug_report << endl;
        }
    }
    else
    {
        cout << "Error: parameter not enough" << endl;
        cout << desc << endl;
    }

    return 0;
}
