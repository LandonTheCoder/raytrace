#pragma once
// Pragma once must be at top or else this gets included multiple times.
#include "utils.h"

class interval {
  public:
    double min, max;

    // Interval is empty by default.
    interval(): min(+infinity), max(-infinity) {}

    interval(double min, double max) : min(min), max(max) {}

    double size() const {
        return max - min;
    }

    bool contains(double x) const {
        return min <= x && x <= max;
    }

    bool surrounds(double x) const {
        return min < x && x < max;
    }

    // Defined in interval.c++ so I can use incremental linking safely
    static const interval empty, universe;
};
