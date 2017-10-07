#include <iostream>
#include <functional>
#include <ctime>
#include <string>
#include <fstream>
#include <iomanip>
#include <tuple>
#include <list>
#include <sstream>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/program_options.hpp>

#include "csv.hpp"
#include "json.hpp"

using namespace std;

using BoostPoint = boost::geometry::model::d2::point_xy<double>;
using BoostPolygon = boost::geometry::model::polygon<BoostPoint, false, false>;
using CellID = unsigned long long;

struct
{
    void begin(const string &tag)
    {
        currentTag = tag;
        cout << "\033[1;34m"
             << "["
             << currentTag
             << "]"
             << " ...\033[0m"
             << endl;

        beginTime = clock();
    }

    double end()
    {
        double interval = (clock() - beginTime) * 1.0 / CLOCKS_PER_SEC;
        if (currentTag.size() > 0)
        {
            cout << "\033[A\33[2K\r"
                 << "\033[1;32m"
                 << "["
                 << currentTag
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
            currentTag = "";
        }
        return interval;
    }

  private:
    clock_t beginTime;
    string currentTag;
} timestamp;

struct AOI
{
    const BoostPolygon boostPolygon;
    set<CellID> cells;
};

struct Scene
{
    const BoostPolygon boostPolygon;
    double price;
    set<CellID> cells;
};

struct
{

    auto aois(const string &path)
    {
        auto ret = list<shared_ptr<AOI>>();

        io::CSVReader<1, io::trim_chars<' '>, io::double_quote_escape<',', '\"'>> in(path);
        in.read_header(io::ignore_extra_column, "Polygon");
        string polygon_string;
        while (in.read_row(polygon_string))
        {
            ret.push_back(make_shared<AOI>(AOI{parse_polygon(polygon_string)}));
        }

        return ret;
    }

    auto scenes(const string &path)
    {
        auto ret = list<shared_ptr<Scene>>();

        io::CSVReader<2, io::trim_chars<' '>, io::double_quote_escape<',', '\"'>> in(path);
        in.read_header(io::ignore_extra_column, "Polygon", "Price");

        string polygon_string;
        double price;
        while (in.read_row(polygon_string, price))
        {
            ret.push_back(make_shared<Scene>(Scene{parse_polygon(polygon_string), price}));
        }

        return ret;
    }

  private:
    list<double> parseDoubles(const string &s)
    {
        list<double> ret;
        // required string format:
        // [[0.105, 0.105], [0.2, 0.2], ..., [0.2, 0.2]]

        const auto nexti = [&](const string &s, int &i) {
            while (i < s.size() && !isdigit(s[i]) && s[i] != '.' && s[i] != '-')
                ++i;
        };

        const auto nextdouble = [](const string &s, int &i) {
            unsigned long long digits = 0;
            double decimal = 1;

            bool dot = false;
            int sign = 1;

            if (s[i] == '-')
            {
                sign = -1;
                ++i;
            }

            while (i < s.size())
            {
                if (s[i] == '.')
                {
                    dot = true;
                }
                else if (isdigit(s[i]))
                {
                    digits *= 10;
                    digits += s[i] - '0';
                    if (dot)
                    {
                        decimal *= 10;
                    }
                }
                else
                {
                    ++i; // to avoid an extra check
                    break;
                }
                ++i;
            }
            return digits / decimal * sign;
        };

        int i = 0;
        nexti(s, i);

        while (i < s.size())
        {
            ret.push_back(nextdouble(s, i));
            nexti(s, i);
        }

        return ret;
    }

    list<BoostPoint> parseBoostPoints(const string &s)
    {
        auto doubles = parseDoubles(s);

        list<BoostPoint> points;

        for (auto i = doubles.begin(); i != doubles.end(); ++i)
        {
            double x = *i;
            double y = *(++i);
            points.emplace_back(x, y);
        }

        return points;
    }

    BoostPolygon parse_polygon(const string &s)
    {
        auto ret = BoostPolygon();
        boost::geometry::append(ret, parseBoostPoints(s));
        return ret;
    }

} load;

CellID indexToCellID(int xi, int yi)
{
    return xi + ((CellID)yi << 32);
}

auto selectPossibleScenes(const AOI &aoi, const list<shared_ptr<Scene>> &scenes)
{
    list<shared_ptr<Scene>> possibleScenes;
    copy_if(scenes.begin(), scenes.end(), back_inserter(possibleScenes), [&](const auto &scene) {
        return boost::geometry::intersects(aoi.boostPolygon, scene->boostPolygon);
    });
    return possibleScenes;
}

set<CellID> discretizeBoostPolygon()
{
}

void discretize(AOI &aoi,
                list<shared_ptr<Scene>> &possibleScenes,
                double delta)
{
    auto discretizeBoostPolygon = [delta](const BoostPolygon &boostPolygon, bool aggresive) {
        const auto &vertices = boostPolygon.outer();

        double minx = vertices[0].x();
        double miny = vertices[0].y();
        double maxx = vertices[0].x();
        double maxy = vertices[0].y();

        for (const auto &vertex : vertices)
        {
            minx = min(minx, vertex.x());
            miny = min(miny, vertex.y());
            maxx = max(maxx, vertex.x());
            maxy = max(maxy, vertex.y());
        }

        int minxi, minyi, maxxi, maxyi;
        function<bool(int, int, const BoostPolygon &)> cond;

        if (aggresive)
        {
            minxi = floor(minx / delta);
            minyi = floor(miny / delta);

            maxxi = ceil(maxx / delta);
            maxyi = ceil(maxy / delta);

            cond = [delta](int xi, int yi, const BoostPolygon &boostPolygon) {
                double x = xi * delta;
                double y = yi * delta;

                auto cellBoostPolygon = BoostPolygon();
                boost::geometry::append(cellBoostPolygon,
                                        vector<BoostPoint>{BoostPoint(x, y),
                                                           BoostPoint(x + delta, y),
                                                           BoostPoint(x + delta, y + delta),
                                                           BoostPoint(x, y + delta)});

                return boost::geometry::intersects(cellBoostPolygon,
                                                   boostPolygon);
            };
        }
        else
        {
            minxi = ceil(minx / delta);
            minyi = ceil(miny / delta);

            maxxi = floor(maxx / delta);
            maxyi = floor(maxy / delta);

            cond = [delta](int xi, int yi, const BoostPolygon &boostPolygon) {
                double x = xi * delta;
                double y = yi * delta;

                for (int i = 0; i < 2; ++i)
                {
                    for (int j = 0; j < 2; ++j)
                    {
                        if (!boost::geometry::within(BoostPoint(x + i * delta, y + j * delta), boostPolygon))
                        {
                            return false;
                        }
                    }
                }
                return true;
            };
        }

        set<CellID> ret;

        for (int i = minxi; i < maxxi; ++i)
        {
            for (int j = minyi; j < maxyi; ++j)
            {
                if (cond(i, j, boostPolygon))
                {
                    ret.insert(indexToCellID(i, j));
                }
            }
        }

        return ret;
    };

    // discretize aoi
    aoi.cells = discretizeBoostPolygon(aoi.boostPolygon, true);

    // discretize scenes
    auto it = possibleScenes.begin();
    while (it != possibleScenes.end())
    {
        auto &scene = *it;

        scene->cells = discretizeBoostPolygon(scene->boostPolygon, false);

        auto cells = scene->cells;
        scene->cells.clear();

        set_intersection(cells.begin(),
                         cells.end(),
                         aoi.cells.begin(),
                         aoi.cells.end(),
                         inserter(scene->cells, scene->cells.begin()));

        if (scene->cells.size() == 0)
        {
            possibleScenes.erase(it++);
        }
        else
        {
            it++;
        }
    }
}

auto greedyOptimize(AOI &aoi,
                    list<shared_ptr<Scene>> &possibleScenes)
{
    list<shared_ptr<Scene>> resultScenes;

    unsigned int covered = 0;
    unsigned int nPossibleScenes = possibleScenes.size();

    while (covered < aoi.cells.size() && resultScenes.size() < nPossibleScenes)
    {
        auto it = min_element(possibleScenes.begin(), possibleScenes.end(), [](const auto &a, const auto &b) {
            return a->price / a->cells.size() < b->price / b->cells.size();
        });

        auto scene = *it;
        possibleScenes.erase(it);

        covered += scene->cells.size();

        for (auto &possibleScene : possibleScenes)
        {
            for (const auto &cell : scene->cells)
            {
                possibleScene->cells.erase(cell);
            }
        }

        resultScenes.push_back(scene);
    }

    return resultScenes;
}

struct
{
    bool enableCell;
    bool enablePolygon;

    auto getScenesReport(const list<shared_ptr<Scene>> &scenes)
    {
        nlohmann::json ret;

        auto getSceneReport = [=](const Scene &scene) {
            nlohmann::json ret;
            if (enablePolygon)
            {
                ret["polygon"] = boostPolygonToString(scene.boostPolygon);
            }
            if (enableCell)
            {
                ret["cells"] = scene.cells;
            }
            ret["price"] = scene.price;
            return ret;
        };

        for (const auto &scene : scenes)
        {
            ret.push_back(getSceneReport(*scene));
        }

        return ret;
    };

    auto getAOIReport(const AOI &aoi)
    {
        nlohmann::json ret;
        if (enablePolygon)
        {
            ret["polygon"] = boostPolygonToString(aoi.boostPolygon);
        }
        if (enableCell)
        {
            ret["cells"] = aoi.cells;
        }
        return ret;
    };

    auto calculateCoverageRatio(const AOI &aoi, const list<shared_ptr<Scene>> &scenes)
    {
        // copy new boostPolygons to union
        list<BoostPolygon> sceneBoostPolygons;
        transform(scenes.begin(), scenes.end(), back_inserter(sceneBoostPolygons), [](const auto &scene) {
            return scene->boostPolygon;
        });

        const auto unionIteration = [](list<BoostPolygon> &boostPolygons) {
            for (auto i = boostPolygons.begin(); i != boostPolygons.end(); ++i)
            {
                for (auto j = next(i); j != boostPolygons.end(); ++j)
                {
                    const auto &boostPolygon1 = *i;
                    const auto &boostPolygon2 = *j;

                    if (boost::geometry::intersects(boostPolygon1, boostPolygon2)) // only try two merge two polygon when they intersects
                    {
                        list<BoostPolygon> unionedPolygons;
                        boost::geometry::union_(boostPolygon1, boostPolygon2, unionedPolygons);
                        boostPolygons.erase(i);
                        boostPolygons.erase(j);
                        boostPolygons.splice(boostPolygons.end(), unionedPolygons);
                        return true;
                    }
                }
            }
            return false;
        };

        // input list will keep shrinking until finish
        while (unionIteration(sceneBoostPolygons))
            ;

        double coverageArea = accumulate(sceneBoostPolygons.begin(), sceneBoostPolygons.end(), 0, [&](int acc, const auto &boostPolygon) {
            list<BoostPolygon> intersectionPolygons;
            boost::geometry::intersection(aoi.boostPolygon, boostPolygon, intersectionPolygons);
            return acc + accumulate(intersectionPolygons.begin(), intersectionPolygons.end(), 0, [](int acc, const auto &boostPolygon) {
                       return acc + boost::geometry::area(boostPolygon);
                   });
        });

        return coverageArea / boost::geometry::area(aoi.boostPolygon);
    };


  private:
    string boostPolygonToString(const auto &boostPolygon)
    {
        const auto &vertices = boostPolygon.outer();
        ostringstream oss;

        oss << "[";
        for (int i = 0; i < vertices.size(); ++i)
        {
            if (i)
            {
                oss << ", ";
            }
            oss << "[" << vertices[i].x() << ", " << vertices[i].y() << "]";
        }
        oss << "]";

        return oss.str();
    };

} analysis;

auto query(AOI &aoi, const list<shared_ptr<Scene>> &scenes, double delta)
{
    nlohmann::json report;
    analysis.enableCell = true;
    analysis.enablePolygon = true;
    
    timestamp.begin("t1");
    auto possibleScenes = selectPossibleScenes(aoi, scenes);
    discretize(aoi, possibleScenes, delta);
    double t1 = timestamp.end();
    
    report["aoi"] = analysis.getAOIReport(aoi);
    report["possibleScenes"] = analysis.getScenesReport(possibleScenes);

    timestamp.begin("t2");
    auto resultScenes = greedyOptimize(aoi, possibleScenes);
    double t2 = timestamp.end();

    report["resultScenes"] = analysis.getScenesReport(resultScenes);
    report["coverageRatio"] = analysis.calculateCoverageRatio(aoi, resultScenes);
    report["timestamp"] = {{"t1", t1}, {"t2", t2}};
    report["delta"] = delta;
    return report;
}

void experiment(double delta, const string &aoisPath, const string &scenesPath, const string &outPath)
{
    auto jsonReport = nlohmann::json();

    timestamp.begin("loading");
    const auto scenes = load.scenes(scenesPath);
    const auto aois = load.aois(aoisPath);
    timestamp.end();

    for (auto &aoi : aois)
    {
        jsonReport.push_back(query(*aoi, scenes, delta));
    }

    ofstream ofs(outPath);
    ofs << jsonReport << endl;
}

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
        experiment(vm["delta"].as<double>(), vm["aoi-path"].as<string>(), vm["scenes-path"].as<string>(), vm["output-path"].as<string>());
    }
    else
    {
        cout << desc << endl;
    }

    return 0;
}
