#ifndef CGSC_SOLVER_H
#define CGSC_SOLVER_H

#include <vector>
#include <iostream>
#include <cstdlib>

#include "transformer.h"
#include "optimizer.h"

class Solver
{
public:
  Solver(std::shared_ptr<Transformer> transformer, std::shared_ptr<Optimizer> optimizer);
  nlohmann::json solve(const Roi &roi, const Products &products, Products &&result_products) const;
  std::string tag() const;

private:
  std::shared_ptr<Transformer> transformer;
  std::shared_ptr<Optimizer> optimizer;
};

#endif