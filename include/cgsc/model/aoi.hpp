#ifndef CGSC_MODEL_AOI_HPP
#define CGSC_MODEL_AOI_HPP

#include <string>
#include <vector>
#include <memory>

#include <boost/geometry.hpp>

#include "polygon.hpp"
#include "grid.hpp"

namespace cgsc
{
namespace model
{
class AOI : public Polygon
{
  public:
    AOI(const std::vector<Point> &vertices)
        : Polygon(vertices)
    {
        area = boost::geometry::area(boostPolygon);
    }

    AOI(const std::string &s)
        : AOI(parsePoints(s))
    {
    }

    void setDelta(double delta)
    {
        this->delta = delta;

        updateGrids();
    }

    std::vector<std::shared_ptr<Grid>> getGrids() const
    {
        return grids;
    }

    double getArea() const
    {
        return area;
    }

  private:
    void updateGrids()
    {
        const auto &vertices = boostPolygon.outer();

        double minx, miny, maxx, maxy;

        for (const auto &vertex : vertices)
        {
            minx = std::min(minx, vertex.x());
            miny = std::min(miny, vertex.y());
            maxx = std::max(maxx, vertex.x());
            maxy = std::max(maxy, vertex.y());
        }

        int minxi = std::floor(minx / delta);
        int minyi = std::floor(miny / delta);

        int maxxi = std::ceil(maxx / delta);
        int maxyi = std::ceil(maxy / delta);

        for (int i = minxi; i < maxxi; ++i)
        {
            for (int j = minyi; j < maxyi; ++j)
            {
                auto grid = make_shared<Grid>(i, j, delta);
                if (overlaps(*grid))
                {
                    grids.push_back(grid);
                }
            }
        }
    }

  private:
    double area;
    double delta;
    std::vector<std::shared_ptr<Grid>> grids;
};
}
}

#endif