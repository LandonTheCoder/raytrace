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
