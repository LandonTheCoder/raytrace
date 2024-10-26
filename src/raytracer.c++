#include <rt/rtweekend.h>

// Camera is responsible for doing a render, given the set parameters.
#include <rt/camera.h>
// hit_record and hittable abstract base class
#include <rt/hittable.h>
// List of hittable objects.
#include <rt/hittable-list.h>
// Includes the types of material
#include <rt/material.h>
// The sphere (which is currently the only hittable)
#include <rt/sphere.h>
// For bitmap class
#include <rt/bitmap.h>
// For struct args and argument parser.
#include <rt/args.h>
// OS-specific workarounds/quirks
#include <rt/quirks.h>

#include <iostream>
// For std::ofstream
#include <fstream>
// For signal handler (to delete the file upon exit)
#include <csignal>
// For std::filesystem::path (to handle deleting it)
#include <filesystem>

using namespace rt;

static std::filesystem::path fpath;
static std::ofstream out_file;

int main(int argl, char **args) {
    // Setup quirks to help ensure the environment
    int locale_is_good = ensure_locale();
    // Returns 0 if success/no-op, -1 if unavailable, positive error code otherwise.
    int vt_escape_status = enable_vt_escapes();
    // Select line printer function
    line_printer = vt_escape_status==0?
                   print_lines_remaining_ansi :
                   print_lines_remaining_plain;
    // Select done printer function
    done_printer = vt_escape_status==0? print_done_ansi: print_done_plain;
    // This *should* make sure Windows doesn't clobber binary files written to std::cout.
    fix_stdout();

    struct args pargs = parse_args(argl, args);

    // If not UTF-8, it returns the codepage in locale_is_good
    if (locale_is_good > 0) {
        // Terminal escapes supported, so do fancy print
        // "\e[1m" is bold, "\e[31m" is red, "\e[0m" is reset
        char acperr_ansi[] = "\033[1;31mWARNING\033[0m: Not running in UTF-8 mode! Non-ASCII "
                             "filenames may fail to open! Running in codepage ";
        // The boring version
        char acperr_plain[] = "WARNING: Not running in UTF-8 mode! Non-ASCII "
                              "filenames may fail to open! Running in codepage ";
        std::clog << (vt_escape_status == 0? acperr_ansi : acperr_plain)
                  << locale_is_good << '\n';

        // Experimental: Try to reconvert filename
        char *temp = reconv_cli_arg(pargs.fname_pos, argl, pargs.fname);
        if (temp == nullptr) {
            std::clog << "Filename conversion failed.\n";
            return 6;
        }
        pargs.fname = temp;
    }

    // We open file here so errors with opening are found early.
    if (pargs.fname != nullptr) {
        // Binary is to keep Windows from changing 0x0A to {0x0D, 0x0A} in a binary file
        // Check if fname is UTF-8 on (current builds of) Windows.
        out_file.open(pargs.fname, std::ios_base::out
                                   | std::ios_base::binary
                                   | std::ios_base::trunc);
        fpath = std::filesystem::path(pargs.fname);
        // It doesn't let me use closures :(
        // I want to delete the empty file when I exit.
        std::signal(SIGINT, [](int signum) -> void {
            // Note: This is because it seems I *have* to close before deleting on Windows.
            out_file.close();
            std::filesystem::remove(fpath);
            std::exit(0);
        });
    }

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
    auto raw_bmp = cam.render(world, pargs.n_threads);

    // This is a trick to avoid writing the code twice for stdout and a file.
    std::ostream &outstream = (pargs.fname != nullptr)? out_file : std::cout;

    if (pargs.ftype == BMPOUT_PPM) {
        raw_bmp.write_as_ppm(outstream);
    } else if (pargs.ftype == BMPOUT_BMP) {
        // The officially preferred way to write BMP is bottom-to-top row order.
        raw_bmp.write_as_bmp_btt(outstream);
    } else if (pargs.ftype == BMPOUT_PNG) {
        // I need to test if this works.
        raw_bmp.write_as_png(outstream);
    } else if (pargs.ftype == BMPOUT_JPEG) {
        raw_bmp.write_as_jpeg(outstream);
    }
}
