#pragma once

#include <cmath>
#include <iostream>
// For random_double()
#include "utils.h"

namespace rt {

class vec3 {
  public:
    // By convention, colors are floats from 0.0-1.0.
    double e[3];

    // Constructors
    vec3() : e{0,0,0} {}
    vec3(double e0, double e1, double e2) : e{e0, e1, e2} {}

    // x/y/z mappings (only as function call)
    double x() const { return e[0]; }
    double y() const { return e[1]; }
    double z() const { return e[2]; }

    vec3 operator -() const { return vec3(-e[0], -e[1], -e[2]); }
    double operator [](int i) const { return e[i]; }
    double & operator [](int i) { return e[i]; }

    vec3 & operator +=(const vec3 &v) {
        e[0] += v.e[0];
        e[1] += v.e[1];
        e[2] += v.e[2];
        return *this;
    }

    vec3 & operator *=(double t) {
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        return *this;
    }

    vec3 & operator /=(double t) {
        return *this *= 1/t;
    }

    double length() const {
        return std::sqrt(length_squared());
    }

    double length_squared() const {
        return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
    }

    bool near_zero() const {
        // Return true if vector is close to 0 in all dimensions
        auto s = 1e-8;
        return (std::fabs(e[0]) < s) && (std::fabs(e[1]) < s) && (std::fabs(e[2]) < s);
    }

    // These are used for generating random directions for diffuse objects.
    static vec3 random() {
        return vec3(random_double(), random_double(), random_double());
    }

    static vec3 random(double min, double max) {
        return vec3(random_double(min, max),
                    random_double(min, max),
                    random_double(min, max));
    }
};

// point3 is just an alias for vec3, but useful for geometric clarity in the code.
using point3 = vec3;

}

// Vector Utility Functions

inline std::ostream & operator <<(std::ostream &out, const rt::vec3 &v) {
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline rt::vec3 operator +(const rt::vec3 &u, const rt::vec3 &v) {
    return rt::vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

inline rt::vec3 operator -(const rt::vec3 &u, const rt::vec3 &v) {
    return rt::vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

inline rt::vec3 operator *(const rt::vec3 &u, const rt::vec3 &v) {
    return rt::vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

inline rt::vec3 operator *(double t, const rt::vec3 &v) {
    return rt::vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}

inline rt::vec3 operator *(const rt::vec3 &v, double t) {
    return t * v;
}

inline rt::vec3 operator /(const rt::vec3 &v, double t) {
    return (1/t) * v;
}

inline double dot(const rt::vec3 &u, const rt::vec3 &v) {
    return u.e[0] * v.e[0]
           + u.e[1] * v.e[1]
           + u.e[2] * v.e[2];
}

inline rt::vec3 cross(const rt::vec3 &u, const rt::vec3 &v) {
    return rt::vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
                    u.e[2] * v.e[0] - u.e[0] * v.e[2],
                    u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

inline rt::vec3 unit_vector(const rt::vec3 &v) {
    return v / v.length();
}

inline rt::vec3 random_unit_vector() {
    while (true) {
        auto p = rt::vec3::random(-1, 1);
        auto len_squared = p.length_squared();

        // Very small values of len_squared can underflow to 0, 10^-160 is smallest safe value.
        if (1e-160 < len_squared && len_squared <= 1)
            return p / sqrt(len_squared);
    }
}

inline rt::vec3 random_in_unit_disk() {
    while (true) {
        auto p = rt::vec3(rt::random_double(-1, 1), rt::random_double(-1, 1), 0);
        if (p.length_squared() < 1)
            return p;
    }
}

inline rt::vec3 random_on_hemisphere(const rt::vec3 &normal) {
    rt::vec3 on_unit_sphere = random_unit_vector();
    // In same hemisphere as normal
    if (dot(on_unit_sphere, normal) > 0.0)
        return on_unit_sphere;
    else
        return -on_unit_sphere;
}

inline rt::vec3 reflect(const rt::vec3 &v, const rt::vec3 &n) {
    // The direction of a reflected ray is (v + 2b), where n is unit vector but v may not be.
    // Vec v points into surface, b points out, so there is a negation (hence - instead of +).
    return v - 2 * dot(v, n) * n;
}

inline rt::vec3 refract(const rt::vec3 &uv, const rt::vec3 &n, double etai_over_etat) {
    // (-R · n), used in R' perpendicular
    auto cos_theta = std::fmin(dot(-uv, n), 1.0);
    // etai_over_etat is η/η', uv is vec R
    rt::vec3 r_out_perp =  etai_over_etat * (uv + cos_theta * n);
    rt::vec3 r_out_parallel = -std::sqrt(std::fabs(1.0 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}
