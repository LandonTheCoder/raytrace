#pragma once

#include "vec3.h"
#include "ray.h"
#include "hittable.h"
#include "color.h"

class camera {
  public:
    // Place public camera parameters here.
    double aspect_ratio = 1.0; // Image width over height
    int image_width = 100; // I thought it was supposed to match main()!

    void render(const hittable &world);
  private:
    // Place private camera variables here.
    int image_height; // Rendered image height
    point3 center; // Camera center
    point3 pixel00_loc; // Location of pixel (0, 0)
    vec3 pixel_delta_u; // Offset to the next pixel to the right
    vec3 pixel_delta_v; // Offset to the next pixel down

    void initialize();
    color ray_color(const ray &r, const hittable &world);

};
