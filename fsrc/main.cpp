#include <iostream>
#include <tuple>
#include <sstream>

#include "csv.hpp"
#include "json.hpp"

using namespace std;

/************************************ Geometry ***********************************************/

// List
template<class T>
using List = vector<T>;

// data Point = (double, double)
using Point = tuple<double, double>;

// data Polygon = [Point]
using Polygon = List<Point>;

/************************************ Model **************************************************/

// data AOI = (Polygon)
using AOI = tuple<Polygon>;

// data Scene = (Polygon, Double)
using Scene = tuple<Polygon, double>;

/*********************************** Load Data ***********************************************/

// String -> Polygon
auto parse_polygon(const string &poly_s)
{
    Polygon ret;
    char br, comma;
    double x, y;
    istringstream iss(poly_s);
    iss >> br; // read [
    while (!iss.eof())
    {
        iss >> br >> x >> comma >> y >> br; // read [1, 2]
        ret.push_back(Point{x, y});
        iss >> comma; // read , or ]

        if (comma == ']')
            break;
    }
    return ret;
}

// String -> IO [Scene]
auto load_scenes(const string &path)
{
    vector<Scene> ret;
    io::CSVReader<2, io::trim_chars<' '>, io::double_quote_escape<',', '\"'>> in(path);
    in.read_header(io::ignore_extra_column, "Polygon", "Price");
    string poly_s;
    double price;
    while (in.read_row(poly_s, price))
        ret.emplace_back(Scene{parse_polygon(poly_s), price});
    return ret;
}

// String -> IO [AOI]
auto load_aois(const string &path)
{
    vector<AOI> ret;
    io::CSVReader<1, io::trim_chars<' '>, io::double_quote_escape<',', '\"'>> in(path);
    in.read_header(io::ignore_extra_column, "Polygon");
    string poly_s;
    while (in.read_row(poly_s))
        ret.emplace_back(AOI{parse_polygon(poly_s)});
    return ret;
}

/************************************ Solution ***********************************************/

// data CellID = unsigned long long
using CellID = unsigned long long;

// data CellSet = [CellID]
using CellSet = List<CellID>;

// data DiscreteAOI = AOI + (CellSet) = (Polygon, CellSet)
using DiscreteAOI = decltype(tuple_cat(AOI(), tuple<CellSet>()));

// data DiscreteScene = Scene + (CellSet) = (Polygon, Double, CellSet)
using DiscreteScene = decltype(tuple_cat(Scene(), tuple<CellSet>()));

// Polygon -> CellSet
auto discretize(const Polygon& polygon)
{
}

/************************************Task Assign***********************************************/

using Report = nlohmann::json;

// AOI -> Report
auto query_experiment_executor_geneartor = ([](path_to_scene, delta, ){
    
    
    return [](const AOI &aoi){
    };
})();

// [AOI] -> [Report]
auto experiment(const List<AOI> &aois)
{
}

int main()
{

}

