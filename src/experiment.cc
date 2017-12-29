#include "experiment.h"

#include <iostream>
#include <fstream>
#include "csv.hpp"
#include "global.h"
#include "solver.h"

using namespace std;
using nlohmann::json;

class Loader
{
  public:
    Loader(const string &rois_dir, const string &products_dir)
        : rois_dir(rois_dir), products_dir(products_dir)
    {
    }

    const auto &load_rois(string roi_type, double roi_ratio, int num)
    {
        ostringstream oss;
        oss << rois_dir << "/" << roi_type << "_" << roi_ratio << ".csv";
        string path = oss.str();

        if (cached_rois.count(path) == 0)
        {
            auto callback = [](const string &polygon) {
                return Roi{parse_polygon(polygon)};
            };
            cached_rois[path] = parse_csv(path, num, callback, "Polygon");
        }

        return cached_rois[path];
    }

    const auto &load_products(int num)
    {
        string path = products_dir + "/archive.csv";

        if (cached_rois.count(path) == 0)
        {
            auto callback = [](const string &polygon, const string &price) {
                return Product{parse_polygon(polygon), stod(price)};
            };
            cached_products[path] = parse_csv(path, num, callback, "Polygon", "Price");
        }

        return cached_products[path];
    }

  private:
    template <class Callback, class... Columns>
    auto parse_csv(string path, int num, Callback callback, Columns... columns)
        -> vector<decltype(callback(columns...)), allocator<decltype(callback(columns...))>>
    {
        using T = decltype(callback(columns...));
        using Collection = vector<T, allocator<T>>;
        Collection objs;
        try
        { // load products
            io::CSVReader<sizeof...(Columns),
                          io::trim_chars<' '>,
                          io::double_quote_escape<',', '\"'>>
                in(path);
            in.read_header(io::ignore_extra_column, columns...);
            // here is very intersting, the same const char* is used
            while (in.read_row(columns...) && objs.size() < num)
            {
                T obj = callback(columns...);
                objs.push_back(move(obj));
            }
            cout << objs.size() << " loaded from path: " << path << endl;
        }
        catch (...)
        {
            cerr << "Error: please check your path: " << path << endl;
            abort();
        }
        return objs;
    }

  private:
    string rois_dir;
    string products_dir;
    map<string, Rois> cached_rois;
    map<string, Products> cached_products;
};

void experiment(const json &settings)
{
    auto reports = json();

    auto loader = Loader(settings["rois_dir"].get<string>(), settings["products_dir"].get<string>());

    const auto &rois = loader.load_rois(settings["roi_type"].get<string>(),
                                        settings["roi_ratio"].get<double>(),
                                        settings["num_rois"].get<int>());

    const auto &products = loader.load_products(settings["archive_size"].get<int>());

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
        ofstream ofs(settings["output_path"].get<string>());
        ofs << reports << endl;
    }
}
