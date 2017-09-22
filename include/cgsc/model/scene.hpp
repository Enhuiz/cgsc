#ifndef CGSC_MODEL_REGION_HPP
#define CGSC_MODEL_REGION_HPP

#include <string>
#include <vector>

#include <boost/geometry.hpp>

#include "polygon.hpp"

namespace cgsc
{
namespace model
{

class Scene : public Polygon
{
  public:
    Scene(const std::vector<Point> &vertices, double price = 0)
        : Polygon(vertices), price(price)
    {
        area = boost::geometry::area(boostPolygon);
    }

    Scene(const std::string &s, double price)
        : Polygon(s), price(price)
    {
    }

    double getArea() const
    {
        return area;
    }

    double getPrice() const
    {
        return price;
    }

    int getGridCount() const
    {
        return grids.size();
    }

    void setGrids(const std::vector<std::shared_ptr<const Grid>> &grids)
    {
        for (const auto &grid : grids)
        {
            if (covers(*grid))
            {
                this->grids.push_back(grid);
            }
        }
    }

    std::vector<std::shared_ptr<const Grid>> getGrids() const
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

    // TODO
    // Given an AOI, try to find grids a scene covered
    // To use polymorphic, the smart point maybe needed

  private:
    double price;
    double area;

    std::vector<std::shared_ptr<const Grid>> grids;
};
}
}

#endif