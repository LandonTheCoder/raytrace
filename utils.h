#pragma once
// Used for defining constants
#include <limits>

// Constants
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility functions
static inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}
