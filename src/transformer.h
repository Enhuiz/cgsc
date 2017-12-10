#ifndef CGSC_TRANSFORMER_H
#define CGSC_TRANSFORMER_H

#include "model.h"

class Transformer
{
public:
  virtual std::string tag() = 0;
  nlohmann::json transform(const Roi &roi,
                           const Products &products,
                           Range &universe,
                           Ranges &ranges);

private:
  virtual void transform_impl(const Roi &roi,
                              const Products &products,
                              Range &universe,
                              Ranges &ranges,
                              nlohmann::json &report) = 0;
};

class OnlineTranformer : public Transformer
{
public:
  std::string tag() { return "geometric"; }

private:
  void transform_impl(const Roi &roi,
                      const Products &products,
                      Range &universe,
                      Ranges &ranges,
                      nlohmann::json &report);
};

class DiscreteTransformer : public Transformer
{
public:
  DiscreteTransformer(double delta);
  std::string tag() { return "discrete"; }

private:
  void transform_impl(const Roi &roi,
                      const Products &products,
                      Range &universe,
                      Ranges &ranges,
                      nlohmann::json &report);
  std::unordered_set<int> discretize(const Polygon &polygon, std::function<bool(const Polygon &)>);
  int hash(const Polygon &grid_cell);

private:
  double delta;
};

class ContinuousTransformer : public Transformer
{
public:
  std::string tag() { return "continuous"; }

private:
  void transform_impl(const Roi &roi,
                      const Products &products,
                      Range &universe,
                      Ranges &ranges,
                      nlohmann::json &report);
};

#endif