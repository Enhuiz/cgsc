#ifndef CGSC_TRANSFORMER_H
#define CGSC_TRANSFORMER_H

#include "model.h"

class Transformer
{
public:
  virtual std::string tag() const = 0;
  nlohmann::json transform(const Roi &roi,
                           const Products &products,
                           Universe &universe,
                           Ranges &ranges) const;

protected:
  void add_imagery_cell(Universe &universe) const;

private:
  virtual void transform_impl(const Roi &roi,
                              const Products &products,
                              Universe &universe,
                              Ranges &ranges) const = 0;

protected:
  mutable nlohmann::json report;
};

class OnlineTranformer : public Transformer
{
public:
  std::string tag() const { return "geometric"; }

private:
  void transform_impl(const Roi &roi,
                      const Products &products,
                      Universe &universe,
                      Ranges &ranges) const;
};

class DiscreteTransformer : public Transformer
{
public:
  DiscreteTransformer(double delta);
  std::string tag() const { return "discrete"; }

private:
  void transform_impl(const Roi &roi,
                      const Products &products,
                      Universe &universe,
                      Ranges &ranges) const;
  std::unordered_set<int> discretize(const Polygon &polygon,
                                     std::function<bool(const Polygon &)>) const;
  int unique_index(const Polygon &grid_cell) const;

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
                      Universe &universe,
                      Ranges &ranges) const;
};

class FastContinuousTransformer : public Transformer
{
public:
  std::string tag() const { return "fast_continuous"; }

private:
  void transform_impl(const Roi &roi,
                      const Products &products,
                      Universe &universe,
                      Ranges &ranges) const;
};

#endif