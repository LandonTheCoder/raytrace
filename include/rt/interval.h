#pragma once
// Pragma once must be at top or else this gets included multiple times.
#include "utils.h"

namespace rt {

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

    double clamp(double x) const {
        if (x < min) return min;
        if (x > max) return max;
        return x;
    }

    // Defined in interval.c++ so I can use incremental linking safely
    static const interval empty, universe;
};

}
