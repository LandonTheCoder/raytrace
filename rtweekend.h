// This is the catch-all header for this program.
#pragma once

// Headers the program uses.
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>

// Abbreviations for convenience.
using std::shared_ptr;
using std::make_shared;

// Constants
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility functions
static inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

// Other program headers
#include "color.h"
#include "ray.h"
#include "vec3.h"
