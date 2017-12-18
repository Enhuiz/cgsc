#include "experiment.h"

#include <iostream>
#include <fstream>

#include "csv.hpp"
#include "global.h"
#include "solver.h"

using namespace std;
using nlohmann::json;

struct Loader
{
    Rois rois;
    Products products;

    Loader(const string &rois_path, int num_rois, const string &products_path, int num_scenes)
    {
        try
        { // load roi
            rois.reserve(num_rois);
            io::CSVReader<1, io::trim_chars<' '>, io::double_quote_escape<',', '\"'>> in(rois_path);
            in.read_header(io::ignore_extra_column, "Polygon");
            string poly_s;
            while (in.read_row(poly_s) && rois.size() < num_rois)
            {
                auto roi = Entity();
                roi.polygon = parse_polygon(poly_s);
                rois.push_back(move(roi));
            }
            cout << rois.size() << " rois loaded" << endl;
        }
        catch (...)
        {
            cerr << "Error: please check your path: " << rois_path << endl;
            abort();
        }
        try
        { // load products
            products.reserve(num_scenes);
            io::CSVReader<2, io::trim_chars<' '>, io::double_quote_escape<',', '\"'>> in(products_path);
            in.read_header(io::ignore_extra_column, "Polygon", "Price");
            string poly_s;
            double price;
            while (in.read_row(poly_s, price) && products.size() < num_scenes)
            {
                auto scene = Entity();
                scene.polygon = parse_polygon(poly_s);
                scene.price = price;
                products.push_back(move(scene));
            }
            cout << products.size() << " products loaded" << endl;
        }
        catch (...)
        {
            cerr << "Error: please check your path: " << products_path << endl;
            abort();
        }
    }
};

string rois_path(const string &dir, const json &s)
{
    ostringstream oss;
    if (s["debug"])
    {
        oss << dir << "/debug.csv";
    }
    else
    {
        oss << dir << "/" << s["roi_type"].get<string>() << "_" << s["roi_ratio"].get<double>() << ".csv";
    }
    return oss.str();
}

string products_path(const string &dir, const json &s)
{
    ostringstream oss;
    if (s["debug"])
    {
        oss << dir << "/debug.csv";
    }
    else
    {
        oss << dir << "/archive.csv";
    }
    return oss.str();
}

void experiment(const string &rois_dir, const string &products_dir, const std::string &output_path, const json &settings)
{
    auto reports = json();

    auto loader = Loader(rois_path(rois_dir, settings),
                         settings["num_rois"].get<int>(),
                         products_path(products_dir, settings),
                         settings["archive_size"].get<int>());

    const auto &rois = loader.rois;
    const auto &products = loader.products;

    auto discrete_transformer = make_shared<DiscreteTransformer>(settings["delta"].get<double>());
    auto continuous_transformer = make_shared<ContinuousTransformer>();
    auto fast_continuous_transformer = make_shared<FastContinuousTransformer>();
    auto greedy_optimizer = make_shared<GreedyOptimizer>(settings["target_coverage"].get<double>());
    auto bnb_optimizer = make_shared<BnbOptimizer>(settings["target_coverage"].get<double>());

    auto perform = [&products, &reports](const Roi &roi, shared_ptr<Transformer> transformer, shared_ptr<Optimizer> optimizer) {
        auto solver = Solver(transformer, optimizer);
        reports.push_back(solver.solve(roi, products, Products()));
        cout << reports.back().dump(4) << endl;
    };

    int cnt = 0;
    for (const auto &roi : rois)
    {
        cout << "Round " << cnt++ << endl;
        perform(roi, continuous_transformer, nullptr);
        perform(roi, fast_continuous_transformer, greedy_optimizer);
        perform(roi, fast_continuous_transformer, bnb_optimizer);
    }
    {
        ofstream ofs(output_path);
        ofs << reports << endl;
    }
}
