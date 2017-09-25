#include <vector>
#include <string>
#include <memory>

#include "csv.hpp"

#include "cgsc/solver/data.h"
#include "cgsc/model/aoi.h"

using namespace cgsc::model;
using namespace std;

namespace cgsc
{
namespace solver
{
Data::Data(const string &scenesPath, const string &aoiPath)
{
    loadScenes(scenesPath);
    loadAOIs(aoiPath);
}

vector<shared_ptr<Scene>> Data::getScenes() const
{
    return scenes;
}

vector<shared_ptr<AOI>> Data::getAOIs() const
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
    io::CSVReader<1, io::trim_chars<' '>, io::double_quote_escape<',', '\"'>> in(path);
    in.read_header(io::ignore_extra_column, "Polygon");
    string polygon;
    while (in.read_row(polygon) && maxRecords != 0)
    {
        aois.push_back(make_shared<AOI>(polygon));
        --maxRecords;
    }
    cout << aois.size() << " AOIs loaded" << endl;
}
}
}
