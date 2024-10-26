#pragma once
#include "vec3.h"
#include "color.h"
#include "ray.h"

#include "hittable.h"

namespace rt {

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
    metal(const color &albedo, double fuzz): albedo(albedo), fuzz(fuzz < 1? fuzz:1) {}

    bool scatter(const ray &r_in, const hit_record &rec,
                 color &attenuation, ray &scattered) const override;

  private:
    color albedo;
    double fuzz;
};

class dielectric: public material {
  public:
    dielectric(double refraction_index): refraction_index(refraction_index) {}

    bool scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered) const override;

  private:
    // Refractive index in vacuum or air, or ratio of refractive index over that of enclosing media.
    double refraction_index;

    // Approximate value of reflectance for a given angle
    static double reflectance(double cosine, double refraction_index);
};

}
