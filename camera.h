#pragma once

#include "vec3.h"
#include "ray.h"
#include "hittable.h"
#include "color.h"
// To allow returning a bitmap
#include "bitmap.h"

class camera {
  public:
    // Place public camera parameters here.
    // Note that users may change these before running render().
    double aspect_ratio = 1.0; // Image width over height
    int image_width = 100; // This is a default image_width.
    int samples_per_pixel = 10; // Number of random samples per pixel.
    int max_depth = 10; // Maximum ray bounces into scene (to limit recursion)

    double vfov = 90; // Vertical view angle/field of view (in degrees)
    point3 lookfrom = point3(0, 0, 0); // Point which camera looks from
    point3 lookat = point3(0, 0, -1); // Point which camera looks at
    vec3 vup = vec3(0, 1, 0); // Camera-relative "up" direction


    double defocus_angle = 0; // Variation angle of rays through each pixel
    double focus_dist = 10; // Distance from camera's lookfrom to plane of perfect focus

    bitmap render(const hittable &world);
  private:
    // Place private camera variables here.
    int image_height; // Rendered image height
    double pixel_samples_scale; // Color scaling factor for sum of samples.
    point3 center; // Camera center
    point3 pixel00_loc; // Location of pixel (0, 0)
    vec3 pixel_delta_u; // Offset to the next pixel to the right
    vec3 pixel_delta_v; // Offset to the next pixel down
    vec3 u, v, w; // Frame-basis vectors for camera
    vec3 defocus_disk_u; // Defocus disk horiz. radius
    vec3 defocus_disk_v; // Defocus disk vert. radius

    void initialize();
    color ray_color(const ray &r, int depth, const hittable &world);

    ray get_ray(int i, int j);
    vec3 sample_square() const;
    point3 defocus_disk_sample() const;
};
