/*
 * PiecewiseSLRFilter.h
 *
 *  Created on: Nov 11, 2020
 *      Author: markus
 */

#ifndef INET_NETWORKLAYER_SERUM_PIECEWISESLRFILTER_H_
#define INET_NETWORKLAYER_SERUM_PIECEWISESLRFILTER_H_

#include <deque>

namespace inet {

class PiecewiseSLRFilter {

public:

    PiecewiseSLRFilter(const unsigned long size = 10, const double triggerWindowFraction = 0.4, const double relativeErrorThreshold = 0.05, const double maxValue = 0);

    double filter(const double value);
    double filtered() const;
    double averageError() const;
    double raw() const;
    operator double() const;

private:

    struct LinearFit {
        double alpha;
        double beta;
    };

    LinearFit computeFit() const;
    double computeValue(const LinearFit &fit, const double x) const;

    std::deque<double> buf = { };
    unsigned long size;
    double triggerWindowFraction;
    double relativeErrorThreshold;
    bool autoDetectMaxValue;
    double maxValue;
    double lastValue;
    LinearFit lastFit;
    double lastAverageAD;

};

} // namespace

#endif /* INET_NETWORKLAYER_SERUM_PIECEWISESLRFILTER_H_ */
