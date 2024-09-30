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

    auto ground_material = make_shared<lambertian>(color(.5, .5, .5));
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            // a is base point for x, b is base point for z, with constant y.
            auto choose_mat = random_double();
            point3 center(a + .9 * random_double(), .2, b + .9 * random_double());

            if ((center - point3(4, .2, 0)).length() > .9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < .8) {
                    // Diffuse/Lambertian surface
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    world.add(make_shared<sphere>(center, .2, sphere_material));
                } else if (choose_mat < .95) {
                    // Metal surface
                    auto albedo = color::random(.5, 1);
                    auto fuzz = random_double(0, .5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, .2, sphere_material));
                } else {
                    // Glass surface
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, .2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(.4, .2, .1));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(.7, .6, .5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    // Camera

    camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 1200;
    // Note: This is a really high quality setting that makes it take forever.
    // It was at 100 previously, perhaps 50 would be good for testing?
    cam.samples_per_pixel = 500;
    cam.max_depth = 50;

    // PoV settings
    cam.vfov = 20;
    cam.lookfrom = point3(13, 2, 3);
    cam.lookat = point3(0, 0, 0);
    cam.vup = vec3(0, 1, 0);

    // Defocus settings
    cam.defocus_angle = .6;
    cam.focus_dist = 10.0;

    // Note: +x is right, +y is up, +z is outwards relative to camera.

    // Initializes camera, renders, writes a PPM to stdout. (Make it more flexible in the future.)
    cam.render(world);
}
