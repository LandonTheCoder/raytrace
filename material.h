#pragma once
#include "vec3.h"
#include "color.h"
#include "ray.h"

#include "hittable.h"

// Abstract class for materials
class material {
  public:
    virtual ~material() = default;

    virtual bool scatter(const ray &r_in,
                         const hit_record &rec,
                         color &attenuation,
                         ray &scattered) const {
        return false;
    }
};
