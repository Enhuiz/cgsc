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

    bool covers(const Grid& grid) const 
    {
        for (const Point& point : gird.boostPolygon.outer()) 
        {
            if(!contains(point)) {
                return false;
            }
        }
        return true;
    }

  private:
    double price;
    double area;
};
}
}

#endif