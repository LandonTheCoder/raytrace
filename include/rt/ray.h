#pragma once

#include "vec3.h"

namespace rt {
class ray {
  public:
    ray() {}

    ray(const point3 &origin, const vec3 &direction) : orig(origin), dir(direction) {}

    const point3 & origin() const { return orig; }
    const vec3 & direction() const { return dir; }

    // t is a point along the ray.
    point3 at(double t) const {
        return orig + t * dir;
    }

  private:
    // Origin
    point3 orig;
    // Direction
    vec3 dir;
};

}
