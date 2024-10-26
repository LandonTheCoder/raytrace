#pragma once
// Used for defining constants
#include <limits>
// Currently used for std::rand()
#include <cstdlib>

namespace rt {

// Constants
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility functions
inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

inline double random_double() {
    // rand() returns an integer between 0 and RAND_MAX
    return std::rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
    // Random real between min, max inclusive
    return min + (max - min) * random_double();
}

}
