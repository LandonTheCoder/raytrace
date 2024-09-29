#include "camera.h"
#include <iostream>
// For bitmap class
#include "color.h"
// For random_double()
#include "utils.h"
// For material to scatter
#include "material.h"

void camera::render(const hittable &world) {
    initialize();
    // Fill in with code from main()
    auto raw_bmp = bitmap(image_width, image_height);

    // Go through image from left-to-right, top-to-bottom.
    for (int j = 0; j < image_height; j++) {
        std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
        for (int i = 0; i < image_width; i++) {
            color pixel_color(0, 0, 0);
            for (int sample = 0; sample < samples_per_pixel; sample++) {
                ray r = get_ray(i, j);
                pixel_color += ray_color(r, max_depth, world);
            }

            raw_bmp.write_pixel_vec3(j, i, pixel_samples_scale * pixel_color);
        }
    }
    raw_bmp.write_as_ppm(std::cout);

    // Insert something better here?
    std::clog << "\rDone.                 \n";
}

// Initialize variables
void camera::initialize() {
    image_height = int(image_width/aspect_ratio);
    image_height = (image_height < 1)? 1: image_height;

    // We are averaging out the pixel color based on n samples.
    pixel_samples_scale = 1.0 / samples_per_pixel;

    center = lookfrom;

    // Determine dimensions of viewport.
    auto focal_length = (lookfrom - lookat).length();
    auto theta = degrees_to_radians(vfov);
    auto h = std::tan(theta/2);
    // Viewport is the region through which the scene rays pass.
    auto viewport_height = 2.0 * h * focal_length;
    auto viewport_width = viewport_height * (double(image_width)/image_height);

    // Find frame-basis vectors for camera-coordinate frame
    w = unit_vector(lookfrom - lookat);
    u = unit_vector(cross(vup, w));
    v = cross(w, u);

    // Calculate the vectors across the horizontal and down the vertical viewport edges.
    // Viewport_u is left-to-right, viewport_v is top-to-bottom.
    vec3 viewport_u = viewport_width * u; // Vector across horiz. edge
    vec3 viewport_v = viewport_height * -v; // Vector down vert. edge (y-axis is inverted)

    // Calculate the horizontal and vertical delta vectors from pixel to pixel.
    pixel_delta_u = viewport_u / image_width;
    pixel_delta_v = viewport_v / image_height;

    // Calculate the location of the upper left pixel.
    auto viewport_upper_left = center - (focal_length * w) - viewport_u/2 - viewport_v/2;
    pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
}

ray camera::get_ray(int i, int j) {
    /* We build a camera ray which originates from origin and is directed at a
     * randomly sampled point near pixel (i, j)
     */
    auto offset = sample_square();

    auto pixel_sample = (pixel00_loc
                         + ((i + offset.x()) * pixel_delta_u)
                         + ((j + offset.y()) * pixel_delta_v));

    auto ray_origin = center;
    auto ray_direction = pixel_sample - ray_origin;

    return ray(ray_origin, ray_direction);
}

vec3 camera::sample_square() const {
    // Returns vector to a random point in the square encompassing ([-.5, .5], [-.5, .5])
    return vec3(random_double() - 0.5, random_double() - 0.5, 0);
}

// At a = 0 it is white, at a = 1.0 it is blue, blend in between.
color camera::ray_color(const ray &r, int depth, const hittable &world) {
    // Don't gather any more light if max depth is exceeded
    if (depth <= 0)
        return color(0, 0, 0);

    hit_record rec;

    // Ignore really close hits to hack around "shadow acne" problem
    if (world.hit(r, interval(0.001, infinity), rec)) {
        ray scattered;
        color attenuation;
        // Recurse until it stops hitting something or exceeds max depth.
        if (rec.mat->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, depth - 1, world);
        return color(0, 0, 0);
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5 * (unit_direction.y() + 1.0);
    // This is a linear interpolation.
    return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
}
