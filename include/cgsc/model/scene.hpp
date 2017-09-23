#ifndef CGSC_MODEL_REGION_HPP
#define CGSC_MODEL_REGION_HPP

#include <string>
#include <list>
#include <set>

#include <boost/geometry.hpp>

#include "polygon.hpp"

namespace cgsc
{
namespace model
{

class Scene : public Polygon
{
  public:
    Scene(const std::list<Point> &vertices, double price = 0)
        : Polygon(vertices), price(price)
    {
    }

    Scene(const std::string &s, double price)
        : Scene(parseListOf<Point>(s), price)
    {
    }

    double getPrice() const
    {
        return price;
    }

    int getGridCount() const
    {
        return grids.size();
    }

    void setGrids(const std::set<std::shared_ptr<Grid>> &grids)
    {
        for (const auto &grid : grids)
        {
            if (covers(*grid))
            {
                this->grids.insert(grid);
            }
        }
    }

    std::set<std::shared_ptr<Grid>> getGrids() const
    {
        return grids;
    }

  private:
    bool covers(const Grid &grid) const
    {
        for (const Point &point : grid.outer())
        {
            if (!contains(point))
            {
                return false;
            }
        }
        return true;
    }

  private:
    double price;
    std::set<std::shared_ptr<Grid>> grids;
};
}
}

#endif