#include <vector>
#include <string>
#include <memory>

#include "csv.hpp"

#include "cgsc/utils/data.h"
#include "cgsc/model/aoi.h"

using namespace cgsc::model;
using namespace std;

namespace cgsc
{
namespace utils
{
Data::Data(const string &scenesPath, const string &aoiPath)
{
    loadScenes(scenesPath);
    loadAOIs(aoiPath);
}

const vector<shared_ptr<const Scene>> &Data::getScenes() const
{
    return scenes;
}

const vector<shared_ptr<const AOI>> &Data::getAOIs() const
{
    return aois;
}

void Data::loadScenes(const string &path, int maxRecords)
{
    io::CSVReader<2, io::trim_chars<' '>, io::double_quote_escape<',', '\"'>> in(path);
    in.read_header(io::ignore_extra_column, "Polygon", "Price");
    string polygon;
    double price;
    while (in.read_row(polygon, price) && maxRecords != 0)
    {
        scenes.push_back(make_shared<Scene>(polygon, price));
        --maxRecords;
    }
    cout << scenes.size() << " scenes loaded" << endl;
}

void Data::loadAOIs(const string &path, int maxRecords)
{
    io::CSVReader<2, io::trim_chars<' '>, io::double_quote_escape<',', '\"'>> in(path);
    in.read_header(io::ignore_extra_column, "Polygon", "Delta");
    string polygon;
    double delta;
    while (in.read_row(polygon, delta) && maxRecords != 0)
    {
        aois.push_back(make_shared<AOI>(polygon, delta));
        --maxRecords;
    }
    cout << aois.size() << " AOIs loaded" << endl;
}
}
}
