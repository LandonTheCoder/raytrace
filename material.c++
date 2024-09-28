#include "material.h"

// Lambertian scatter
bool lambertian::scatter(const ray &r_in, const hit_record &rec,
                         color &attenuation, ray &scattered) const {
    /* We can either always scatter and attenuate according to reflectance,
     * or we can sometimes scatter P(1-R) with no attenuation, or a mix of both.
     * Here we choose to always scatter.
     */
    auto scatter_direction = rec.normal + random_unit_vector();

    // Catch bad scatter direction
    if (scatter_direction.near_zero())
        scatter_direction = rec.normal;

    scattered = ray(rec.p, scatter_direction);
    attenuation = albedo;
    return true;
}

// Metal scatter
bool metal::scatter(const ray &r_in, const hit_record &rec,
                    color &attenuation, ray &scattered) const {
    vec3 reflected = reflect(r_in.direction(), rec.normal);
    // Implement fuzzy reflection
    reflected = unit_vector(reflected) + (fuzz * random_unit_vector());
    scattered = ray(rec.p, reflected);
    attenuation = albedo;
    // Absorbed if the scatter would be below the surface
    return dot(scattered.direction(), rec.normal) > 0;
}

// Dielectric scatter
bool dielectric::scatter(const ray &r_in, const hit_record &rec,
                         color &attenuation, ray &scattered) const {
    attenuation = color(1.0, 1.0, 1.0);
    double ri = rec.front_face ? (1.0/refraction_index) : refraction_index;

    vec3 unit_direction = unit_vector(r_in.direction());
    double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
    double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

    bool cannot_refract = ri * sin_theta > 1.0;
    vec3 direction;
    if (cannot_refract || reflectance(cos_theta, ri) > random_double())
        direction = reflect(unit_direction, rec.normal);
    else
        direction = refract(unit_direction, rec.normal, ri);

    scattered = ray(rec.p, direction);
    return true;
}

// Dielectric scatter: reflectance. This approximates how much it reflects for a given angle.
double dielectric::reflectance(double cosine, double refraction_index) {
    // Use Schlick's approximation for reflectance
    auto r0 = (1 - refraction_index) / (1 + refraction_index);
    r0 *= r0; // Square it
    return r0 + (1 - r0) * std::pow(1 - cosine, 5);
}
