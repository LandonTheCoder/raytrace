#include "sphere.h"

// The override keyword is only shown in the class definition (in header)
bool sphere::hit(const ray &r, double ray_tmin, double ray_tmax, hit_record &rec) const {
    // oc is (C - Q)
    vec3 oc = center - r.origin();
    // a, b (subbed for h), c are the variables from quadratic equation.
    // x.length_squared() is equivalent to x · x
    auto a = r.direction().length_squared();
    auto h = dot(r.direction(), oc); // b but with -2 cancelled out
    auto c = oc.length_squared() - radius * radius;

    // The component inside the square root of quadratic equation.
    // Note that (2b)^2 evaluates to 4b^2.
    auto discriminant = h * h - a * c;
    if (discriminant < 0)
        return false;

    auto sqrtd = std::sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    // root is the quadratic equation result.
    auto root = (h - sqrtd) / a;
    if (root <= ray_tmin || ray_tmax <= root) {
        root = (h + sqrtd) / a;
        if (root <= ray_tmin || ray_tmax <= root)
            return false;
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    rec.normal = (rec.p - center) / radius;

    return true;
}
