#include "color.h"
#include "vec3.h"

#include <iostream>

int main() {

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

            raw_bmp.write_pixel_vec3(j, i, pixel_color);
        }
    }
    raw_bmp.write_as_ppm(std::cout);
    // Insert something better here?
    std::clog << "\rDone.                 \n";
}
