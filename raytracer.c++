#include "color.h"
#include "ray.h"
#include "vec3.h"

#include <iostream>

// Report a normal sphere hit point by math given in NOTES.md.
double hit_sphere(const point3 &center, double radius, const ray &r) {
    // oc is (C - Q)
    vec3 oc = center - r.origin();
    // a, b, c are the variables from quadratic equation.
    auto a = dot(r.direction(), r.direction()); // d · d
    auto b = -2.0 * dot(r.direction(), oc); // -2d · (C - Q)
    auto c = dot(oc, oc) - radius * radius; // (C - Q) · (C - Q) - r^2
    // The component inside the square root of quadratic equation.
    auto discriminant = b * b - 4 * a * c;
    // We square-root in order to normalize normal vector to the unit vector.
    // An outwards normal is direction of (P - C), so pointing out from center.
    if (discriminant < 0) {
        return -1.0;
    } else {
        // For now, we assume the closest hit point is desired.
        return (-b - std::sqrt(discriminant)) / (2.0 * a);
    }
}

// At a = 0 it is white, at a = 1.0 it is blue, blend in between.
color ray_color(const ray &r) {
    // Test a point at (0, 0, -1) and make intersecting pixels red.
    auto t = hit_sphere(point3(0, 0, -1), 0.5, r);
    if (t > 0.0) {
        vec3 N = unit_vector(r.at(t) - vec3(0, 0, -1));
        // Sphere hit colors are between -1 and 0.
        return 0.5 * color(N.x() + 1, N.y() + 1, N.z() + 1);
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5 * (unit_direction.y() + 1.0);
    // This is a linear interpolation.
    return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
}

int main() {

    // Image

    auto aspect_ratio = 16.0 / 9.0;
    int image_width = 400;

    // Calculate the image height, and ensure that it's at least 1.
    int image_height = int(image_width / aspect_ratio);
    image_height = (image_height < 1) ? 1 : image_height;

    // Camera

    auto focal_length = 1.0;
    // Viewport is the region through which the scene rays pass.
    auto viewport_height = 2.0;
    auto viewport_width = viewport_height * (double(image_width)/image_height);
    // This is called the eye point.
    auto camera_center = point3(0, 0, 0);

    // Note: +x is right, +y is up, +z is outwards relative to camera.

    // Calculate the vectors across the horizontal and down the vertical viewport edges.
    // Viewport_u is left-to-right, viewport_v is top-to-bottom.
    auto viewport_u = vec3(viewport_width, 0, 0);
    auto viewport_v = vec3(0, -viewport_height, 0); // y-axis is inverted.

    // Calculate the horizontal and vertical delta vectors from pixel to pixel.
    auto pixel_delta_u = viewport_u / image_width;
    auto pixel_delta_v = viewport_v / image_height;

    // Calculate the location of the upper left pixel.
    auto viewport_upper_left = camera_center
                             - vec3(0, 0, focal_length) - viewport_u/2 - viewport_v/2;
    auto pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

    // Render

    auto raw_bmp = bitmap(image_width, image_height);

    // Go through image from left-to-right, top-to-bottom.
    for (int j = 0; j < image_height; j++) {
        std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
        for (int i = 0; i < image_width; i++) {
            auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
            auto ray_direction = pixel_center - camera_center;
            ray r(camera_center, ray_direction);

            color pixel_color = ray_color(r);
            raw_bmp.write_pixel_vec3(j, i, pixel_color);
        }
    }
    raw_bmp.write_as_ppm(std::cout);
    // Insert something better here?
    std::clog << "\rDone.                 \n";
}
