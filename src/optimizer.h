#ifndef CGSC_OPTIMIZER_H
#define CGSC_OPTIMIZER_H

#include "model.h"

class Optimizer
{
  public:
    Optimizer(double target_coverage);
    virtual nlohmann::json optimize(const Universe &universe, const Ranges &ranges, Ranges &result_ranges) = 0;
    virtual std::string tag() = 0;

  protected:
    double target_coverage; // 0 ~ 1, acceptable value / Universe value
};

class GreedyOptimizer : public Optimizer
{
  public:
    GreedyOptimizer(double target_coverage);
    nlohmann::json optimize(const Universe &universe, const Ranges &ranges, Ranges &result_ranges);
    std::string tag() { return "greedy"; };
};

class BnbOptimizer : public Optimizer
{
  public:
    BnbOptimizer(double target_coverage);
    nlohmann::json optimize(const Universe &universe, const Ranges &ranges, Ranges &result_ranges);
    std::string tag() { return "bnb"; }
};

class OnlineBnbOptimizer : public Optimizer
{
  public:
    OnlineBnbOptimizer(double target_coverage);
    nlohmann::json optimize(const Universe &universe, const Ranges &ranges, Ranges &result_ranges);
    std::string tag() { return "old_bnb"; }
};

#endif