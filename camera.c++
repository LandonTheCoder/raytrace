#include "camera.h"
#include <iostream>
// For bitmap class
#include "color.h"
// For random_double()
#include "utils.h"

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
                pixel_color += ray_color(r, world);
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

    center = point3(0, 0, 0);

    // Determine dimensions of viewport.
    auto focal_length = 1.0;
    // Viewport is the region through which the scene rays pass.
    auto viewport_height = 2.0;
    auto viewport_width = viewport_height * (double(image_width)/image_height);

    // Calculate the vectors across the horizontal and down the vertical viewport edges.
    // Viewport_u is left-to-right, viewport_v is top-to-bottom.
    auto viewport_u = vec3(viewport_width, 0, 0);
    auto viewport_v = vec3(0, -viewport_height, 0); // y-axis is inverted.

    // Calculate the horizontal and vertical delta vectors from pixel to pixel.
    pixel_delta_u = viewport_u / image_width;
    pixel_delta_v = viewport_v / image_height;

    // Calculate the location of the upper left pixel.
    auto viewport_upper_left = center - vec3(0, 0, focal_length) - viewport_u/2 - viewport_v/2;
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
color camera::ray_color(const ray &r, const hittable &world) {
    hit_record rec;
    if (world.hit(r, interval(0, infinity), rec)) {
        vec3 direction = random_on_hemisphere(rec.normal);
        // Is this safe?
        return 0.5 * ray_color(ray(rec.p, direction), world);
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5 * (unit_direction.y() + 1.0);
    // This is a linear interpolation.
    return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
}
