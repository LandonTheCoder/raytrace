#pragma once
#include "hittable.h"
#include "vec3.h"
// For std::shared_ptr
#include <memory>

class sphere: public hittable {
  public:
    sphere(const point3 &center, double radius, std::shared_ptr<material> mat):
        center(center), radius(std::fmax(0, radius)), mat(mat) {}

    bool hit(const ray &r, interval ray_t, hit_record &rec) const override;

  private:
    point3 center;
    double radius;
    std::shared_ptr<material> mat; // To be initialized
};
