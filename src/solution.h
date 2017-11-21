#ifndef CGSC_SOLUTION_H
#define CGSC_SOLUTION_H

#include <list>
#include <iostream>

#include "model.h"

double calculate_density(Entity *roi, const std::list<Entity *>& entities);

template <class Transformer, class Optimizer>
struct Solver
{
    nlohmann::json report;

    Solver(Entity *roi,
           std::list<Entity *> records,
           double target_coverage,
           double delta)
    {
        std::unique_ptr<Transformer> transformer;
        std::unique_ptr<Optimizer> optimizer;

        Stopwatch sw;

        if (Transformer::tag() != "none")
        {
            // select possible records
            sw.restart();
            records = func::filter(records, [roi](Entity *record) {
                return intersects(record->poly, roi->poly);
            });
            sw.pause();
            report["d"] = calculate_density(roi, records);
            report["nop"] = records.size();

            sw.continue_();
            transformer = std::unique_ptr<Transformer>(
                new Transformer(roi,
                                records,
                                delta));

            report["t"] = sw.lap();
            report["noc"] = transformer->universe->elements.size();
            std::cout << report << std::endl;
            logger << "t1 ends after " << sw.lap() << " s" << std::endl;

            if (Optimizer::tag() != "none")
            {
                sw.restart();
                optimizer = std::unique_ptr<Optimizer>(new Optimizer(transformer->universe.get(),
                                                                     func::map(transformer->ranges,
                                                                               [](const std::unique_ptr<Range> &p) {
                                                                                   return p.get();
                                                                               }),
                                                                     target_coverage));
                logger << "t2 ends after " << sw.lap() << " s" << std::endl;
            }
        }
    }

    static std::string tag()
    {
        return Transformer::tag() + "::" + Optimizer::tag();
    }
};

struct Optimizer
{
    nlohmann::json report;
    std::list<Entity *> results;
    Optimizer(){};
    Optimizer(Range *universe, std::list<Range *> ranges, double target_coverage){};
    static std::string tag() { return "none"; };
};

struct Greedy : Optimizer
{
    Greedy(Range *universe, std::list<Range *> ranges, double target_coverage);
    static std::string tag() { return "greedy"; };
};

struct Bnb : Optimizer
{
    Bnb(Range *universe, std::list<Range *> ranges, double target_coverage);
    static std::string tag() { return "bnb"; }
};

#endif