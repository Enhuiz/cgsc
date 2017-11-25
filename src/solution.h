#ifndef CGSC_SOLUTION_H
#define CGSC_SOLUTION_H

#include <list>
#include <iostream>
#include <cstdlib>

#include "model.h"

double calculate_density(const Entity &roi, const std::list<Entity> &entities);

template <class Transformer, class Optimizer>
struct Solver
{
    nlohmann::json report;

    Solver(const Entity &roi,
           const std::list<Entity> &records,
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
            auto possible_records = func::filter(records, [&roi](const Entity &record) {
                return intersects(record.poly, roi.poly);
            });
            sw.pause();
            report["d"] = calculate_density(roi, possible_records);
            report["nop"] = possible_records.size();
            std::cout << possible_records.size() << std::endl;

            sw.continue_();
            transformer = std::unique_ptr<Transformer>(
                new Transformer(roi,
                                possible_records,
                                delta));

            report["histogram"] = transformer->report["histogram"];
            report["t"] = sw.lap();
            report["noc"] = transformer->universe.elements.size();
            std::cout << report << std::endl;
            logger << "t1 ends after " << sw.lap() << " s" << std::endl;

            if (Optimizer::tag() != "none")
            {
                sw.restart();
                optimizer = std::unique_ptr<Optimizer>(new Optimizer(transformer->universe,
                                                                     transformer->ranges,
                                                                     target_coverage));
                logger << "t2 ends after " << sw.lap() << " s" << std::endl;
                report["optimizer"] = optimizer->report;
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
    std::list<std::list<Range>::const_iterator> subranges;
    nlohmann::json report;
    Optimizer(){};
    Optimizer(const Range &universe, const std::list<Range> &ranges, double target_coverage){};
    static std::string tag() { return "none"; };
};

struct Greedy : Optimizer
{
    Greedy(const Range &universe, const std::list<Range> &ranges, double target_coverage);
    static std::string tag() { return "greedy"; };
};

struct Bnb : Optimizer
{
    Bnb(const Range &universe, const std::list<Range> &ranges, double target_coverage);
    static std::string tag() { return "bnb"; }
};

namespace old
{
struct Bnb : Optimizer
{
    Bnb(const Range &universe, const std::list<Range> &ranges, double target_coverage);
    static std::string tag() { return "old_bnb"; }
};
}

#endif