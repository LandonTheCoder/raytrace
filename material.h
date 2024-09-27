#pragma once
#include "vec3.h"
#include "color.h"
#include "ray.h"

#include "hittable.h"

// Abstract class for materials
class material {
  public:
    virtual ~material() = default;

    virtual bool scatter(const ray &r_in, const hit_record &rec,
                         color &attenuation, ray &scattered) const {
        return false;
    }
};

class lambertian: public material {
  public:
    lambertian(const color &albedo): albedo(albedo) {}

    bool scatter(const ray &r_in, const hit_record &rec,
                 color &attenuation, ray &scattered) const override;

  private:
    color albedo;
};

class metal: public material {
  public:
    metal(const color &albedo): albedo(albedo) {}

    bool scatter(const ray &r_in, const hit_record &rec,
                 color &attenuation, ray &scattered) const override;

  private:
    color albedo;
};
