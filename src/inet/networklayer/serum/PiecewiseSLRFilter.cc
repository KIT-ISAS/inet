/*
 * PiecewiseSLRFilter.cc
 *
 *  Created on: Nov 16, 2020
 *      Author: markus
 */

#include "PiecewiseSLRFilter.h"

namespace inet {

PiecewiseSLRFilter::PiecewiseSLRFilter(const unsigned long size, const double triggerWindowFraction, const double relativeErrorThreshold, const double maxValue) :
        size(size), triggerWindowFraction(triggerWindowFraction), relativeErrorThreshold(relativeErrorThreshold),
        autoDetectMaxValue(maxValue >= 0), maxValue(std::abs(maxValue)) {
    ASSERT(size > 0);
    ASSERT(triggerWindowFraction >= 0 && triggerWindowFraction <= 1);
    ASSERT(relativeErrorThreshold >= 0);
}

double PiecewiseSLRFilter::filter(const double value) {
    if (autoDetectMaxValue && value > maxValue) {
        maxValue = value;
    }

    if (buf.size() == size) {
        buf.pop_front();
    }

    buf.push_back(value);

    ulong xMax;
    LinearFit fit;
    double avgAD;
    bool tripped;

    do {
        if (buf.size() < 3) {
            lastValue = value;
            lastAverageAD = 0;

            return value; // no filtering applied
        }

        // compute new alpha and beta for y = alpha + beta * x + epsilon

        fit = computeFit();

        // compute average difference between regression and real values

        xMax = buf.size() - 1;
        avgAD = 0;
        double runningErrSum = 0;
        tripped = false;

        for (long x = xMax; x >= 0; x--) {
            const double filtered = std::max(computeValue(fit, x), 0.0); // suppress negative values
            const double err = filtered - buf[x];

            avgAD += std::abs(err);

            // do not incorporate first sample, since the approximation might
            // cancel out the error introduced by it with the following samples
            ASSERT(x >= 0);
            if ((ulong) x < xMax) {
                runningErrSum += err;
            }

            if ((ulong) x >= std::min(xMax - 1, (ulong) std::floor(buf.size() * (1 - triggerWindowFraction)))) {
                tripped = std::abs(runningErrSum) > maxValue * relativeErrorThreshold;

                if (tripped) {
                    break;
                }
            }
        }

        avgAD /= buf.size();

        if (buf.size() >= 3 && tripped) {
            // halve buffer history and try again
            for (ulong i = buf.size() / 2; i > 0; i--) {
                buf.pop_front();
            }
        }
    } while (tripped);

    // update state
    lastValue = std::min(std::max(computeValue(fit, xMax), 0.0), maxValue); // suppress negative values and values exceeding max range
    lastAverageAD = avgAD;
    lastFit = fit;

    return lastValue;
}

double PiecewiseSLRFilter::filtered() const {
    return lastValue;
}

double PiecewiseSLRFilter::averageError() const {
    return lastAverageAD;
}

double PiecewiseSLRFilter::raw() const {
    return buf.size() > 0 ? buf.back() : 0;
}

PiecewiseSLRFilter::operator double() const {
    return raw();
}

PiecewiseSLRFilter::LinearFit PiecewiseSLRFilter::computeFit() const {
    LinearFit result;

    // compute new alpha and beta for y = alpha + beta * x + epsilon
    const ulong xMax = buf.size() - 1;
    const double sumX = xMax * (xMax + 1) / 2.0;
    const double meanX = sumX / buf.size();
    double meanY = 0;

    for (auto y : buf) {
        meanY += y;
    }

    meanY /= buf.size();

    double spXY = 0;
    double sqX = 0;

    for (ulong x = 0; x <= xMax; x++) {
        const double y = buf[x];

        spXY += (x - meanX) * (y - meanY);
        sqX += (x - meanX) * (x - meanX);
    }

    result.beta = spXY / sqX;
    result.alpha = meanY - result.beta * meanX;

    return result;
}

double PiecewiseSLRFilter::computeValue(const PiecewiseSLRFilter::LinearFit &fit, const double x) const {
    return fit.alpha + fit.beta * x;
}

} // namespace
