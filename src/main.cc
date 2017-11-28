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
    desc.add_options()("rois-dir,r", value<string>(), "roi folder")("archive-dir,a", value<string>(), "archive folder")("setting,s", value<string>(), "setting")("output-path,o", value<string>(), "output path");

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

    if (vm.count("rois-dir") && vm.count("archive-dir") && vm.count("output-path") && vm.count("setting"))
    {
        auto setting = vm["setting"].as<string>();
        auto report = experiment(vm["rois-dir"].as<string>(), vm["archive-dir"].as<string>(), nlohmann::json::parse(setting));
        {
            ofstream ofs(vm["output-path"].as<string>());
            ofs << report << endl;
        }
        {
            // if (debug_report.size() > 0) {
            //     ofstream ofs("../data/visualize/bnb/[with_preproc_v2] " + split(vm["output-path"].as<string>(), "/").back());
            //     ofs << debug_report << endl;
            // }
        }
    }
    else
    {
        cout << "Error: parameter not enough" << endl;
        cout << desc << endl;
    }

    return 0;
}
