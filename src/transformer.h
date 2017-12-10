#ifndef CGSC_TRANSFORMER_H
#define CGSC_TRANSFORMER_H

#include "model.h"

class Transformer
{
public:
  virtual std::string tag() const = 0;
  nlohmann::json transform(const Roi &roi,
                           const Products &products,
                           Range &universe,
                           Ranges &ranges) const;

private:
  virtual void transform_impl(const Roi &roi,
                              const Products &products,
                              Range &universe,
                              Ranges &ranges,
                              nlohmann::json &report) const = 0;
};

class OnlineTranformer : public Transformer
{
public:
  std::string tag() const { return "geometric"; }

private:
  void transform_impl(const Roi &roi,
                      const Products &products,
                      Range &universe,
                      Ranges &ranges,
                      nlohmann::json &report) const;
};

class DiscreteTransformer : public Transformer
{
public:
  DiscreteTransformer(double delta);
  std::string tag() const { return "discrete"; }

private:
  void transform_impl(const Roi &roi,
                      const Products &products,
                      Range &universe,
                      Ranges &ranges,
                      nlohmann::json &report) const;
  std::unordered_set<int> discretize(const Polygon &polygon,
                                     std::function<bool(const Polygon &)>) const;
  int hash(const Polygon &grid_cell) const;

private:
  double delta;
};

class ContinuousTransformer : public Transformer
{
public:
  std::string tag() const { return "continuous"; }

private:
  void transform_impl(const Roi &roi,
                      const Products &products,
                      Range &universe,
                      Ranges &ranges,
                      nlohmann::json &report) const;
};

#endif