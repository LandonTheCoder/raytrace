#include "camera.h"
#include <iostream>
// For bitmap class
#include "color.h"

void camera::render(const hittable &world) {
    initialize();
    // Fill in with code from main()
    auto raw_bmp = bitmap(image_width, image_height);

    // Go through image from left-to-right, top-to-bottom.
    for (int j = 0; j < image_height; j++) {
        std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
        for (int i = 0; i < image_width; i++) {
            auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
            auto ray_direction = pixel_center - center;
            ray r(center, ray_direction);

            color pixel_color = ray_color(r, world);
            raw_bmp.write_pixel_vec3(j, i, pixel_color);
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

// At a = 0 it is white, at a = 1.0 it is blue, blend in between.
color camera::ray_color(const ray &r, const hittable &world) {
    hit_record rec;
    if (world.hit(r, interval(0, infinity), rec)) {
        return 0.5 * (rec.normal + color(1.0, 1.0, 1.0));
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5 * (unit_direction.y() + 1.0);
    // This is a linear interpolation.
    return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
}
