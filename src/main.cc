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
    desc.add_options()("settings,s", value<string>(), "settings");

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

    if (vm.count("settings"))
    {
        auto settings = vm["settings"].as<string>();
        experiment(nlohmann::json::parse(settings));
    }
    else
    {
        cout << "Error: parameter not enough" << endl;
        cout << desc << endl;
    }

    return 0;
}
