#include "rtweekend.h"

#include "camera.h"
#include "hittable.h"
#include "hittable-list.h"
#include "material.h"
#include "sphere.h"

#include <iostream>

int main() {

    // Image size settings are now in camera.h and camera.c++

    hittable_list world;

    auto R = std::cos(pi/4);

    auto material_left = make_shared<lambertian>(color(0, 0, 1));
    auto material_right = make_shared<lambertian>(color(1, 0, 0));

    world.add(make_shared<sphere>(point3(-R, 0, -1), R, material_left));
    world.add(make_shared<sphere>(point3(R, 0, -1), R, material_right));

    // Camera

    camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth = 50;

    cam.vfov = 90;

    // Note: +x is right, +y is up, +z is outwards relative to camera.

    // Initializes camera, renders, writes a PPM to stdout. (Make it more flexible in the future.)
    cam.render(world);
}
