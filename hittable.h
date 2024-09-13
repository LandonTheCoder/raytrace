#pragma once

#include "ray.h"

class hit_record {
  public:
    // Point where it intersected.
    point3 p;
    // Surface normal
    vec3 normal;
    // The t-value which matches.
    double t;
};

// Abstract class to support hittable objects.
class hittable {
  public:
    virtual ~hittable() = default;
    virtual bool hit(const ray &r,
                     double ray_tmin,
                     double ray_tmax,
                     // rec is written to and saves calculation data.
                     hit_record &rec) const = 0;
};
