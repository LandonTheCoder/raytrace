#pragma once

#include "ray.h"
#include "interval.h"
// For std::shared_ptr
#include <memory>


class material;

class hit_record {
  public:
    // Point where it intersected.
    point3 p;
    // Surface normal
    vec3 normal;
    // The t-value which matches.
    double t;
    // Material type, which can implement scattering in different ways.
    std::shared_ptr<material> mat;
    // Is ray facing towards the front?
    bool front_face;
    void set_face_normal(const ray &r, const vec3 &outward_normal) {
        // Set hit record normal vector (outward_normal is assumed to have
        // unit length). Normals always point outwards when stored.
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }

};

// Abstract class to support hittable objects.
class hittable {
  public:
    virtual ~hittable() = default;
    virtual bool hit(const ray &r,
                     interval ray_t,
                     // rec is written to and saves calculation data.
                     hit_record &rec) const = 0;
};
