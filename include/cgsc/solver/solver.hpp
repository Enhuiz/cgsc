#ifndef CGSC_SOLVER_SOLVER_HPP
#define CGSC_SOLVER_SOLVER_HPP

#include <list>
#include <string>
#include <memory>

#include "csv.hpp"
#include "data.hpp"
#include "result.hpp"

#include "../model/aoi.hpp"
#include "data.hpp"

namespace cgsc
{
namespace solver
{
class Solver
{
  public:
    Solver(const std::shared_ptr<Data> &data)
        : data(data)
    {
    }

    virtual Result query(const std::shared_ptr<model::AOI>& aoi) const = 0;

    std::list<Result> getAllResults()
    {
        std::list<Result> ret;
        for (const auto &aoi : data->getAOIs())
        {
            ret.push_back(query(aoi));
        }
        return ret;
    }

protected:
    std::shared_ptr<Data> data;
};
}
}

#endif