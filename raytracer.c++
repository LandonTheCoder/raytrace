#include "rtweekend.h"

#include "camera.h"
#include "hittable.h"
#include "hittable-list.h"
#include "sphere.h"

#include <iostream>

int main() {

    // Image size settings are now in camera.h and camera.c++

    hittable_list world;
    world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
    world.add(make_shared<sphere>(point3(0, -100.5, -1), 100));

    // Camera

    camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 400;
    cam.samples_per_pixel = 100;

    // Note: +x is right, +y is up, +z is outwards relative to camera.

    // Initializes camera, renders, writes a PPM to stdout. (Make it more flexible in the future.)
    cam.render(world);
}
