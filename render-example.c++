#include "color.h"
#include "bitmap.h"
#include "vec3.h"
#include "interval.h"
// For struct args, parse_args()
#include "args.h"

#include <iostream>
// For std::ofstream
#include <fstream>

struct rgb {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

// Converts to RGB in linear space (unlike the bitmap method)
struct rgb color_vec3_to_rgb(const color &pixel_color) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // We now do a clamped translation from [0.0, 1.0] to [0, 255]
    static const interval intensity(0.000, 0.999);

    uint8_t rbyte = uint8_t(256 * intensity.clamp(r));
    uint8_t gbyte = uint8_t(256 * intensity.clamp(g));
    uint8_t bbyte = uint8_t(256 * intensity.clamp(b));

    // Return components
    return {.r = rbyte, .g = gbyte, .b = bbyte};
}

// This is like ppm-example but not hardcoded for ppm to stdout.
int main(int argl, char **args) {
    struct args pargs = parse_args(argl, args);

    std::ofstream out_file;
    if (pargs.fname != nullptr) {
        // Check if this works correctly on UTF-8 clean versions of Windows
        // Binary is needed to ensure it doesn't butcher 0x0A on Windows
        out_file.open(pargs.fname, std::ios_base::out
                                   | std::ios_base::binary
                                   | std::ios_base::trunc);
    }

    // Image

    int image_width = 256;
    int image_height = 256;

    // Render

    auto raw_bmp = bitmap(image_width, image_height);

    // PPM is written left-to-right, top-to-bottom.
    for (int j = 0; j < image_height; j++) {
        std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
        for (int i = 0; i < image_width; i++) {
            auto pixel_color = color(double(i) / (image_width - 1), double(j) / (image_height - 1), 0.0);

            auto pixel_color_rgb = color_vec3_to_rgb(pixel_color);

            // The write_pixel_vec3 does a gamma interpolation, so I convert manually here.
            raw_bmp.write_pixel_rgb(j, i, pixel_color_rgb.r, pixel_color_rgb.g, pixel_color_rgb.b);
        }
    }
    // Handle arguments
    std::ostream &outstream = (pargs.fname != nullptr)? out_file : std::cout;

    if (pargs.ftype == BMPOUT_PPM) {
        raw_bmp.write_as_ppm(outstream);
    } else if (pargs.ftype == BMPOUT_BMP) {
        raw_bmp.write_as_bmp_btt(outstream);
    }

//    raw_bmp.write_as_ppm(std::cout);
    // Insert something better here?
    std::clog << "\rDone.                 \n";
}
