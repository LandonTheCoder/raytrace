#include "vec3.h"
#include "bitmap.h"
// For .bmp writer functions
#include "bmp-format.h"
#include <cstdint>
#include <iostream>
// In case I want to printf()
#include <cstdio>
// For interval.clamp()
#include "interval.h"
// For std::sqrt()
#include <cmath>

// I haven't implemented support for advanced formats yet.
static const BitmapOutputType supported_types[] = {BMPOUT_PPM,
                                                   BMPOUT_BMP,
                                                   BMPOUT_TERMINATOR};

// Global functions

const BitmapOutputType * bitmap::return_supported_types() {
    return supported_types;
}

// Internal function
static inline double linear_to_gamma(double linear_component) {
    if (linear_component > 0)
        return std::sqrt(linear_component);
    return 0;
}

// Private functions for internal usage.

bool bitmap::check_pixel_address_validity(int row, int column) {
    if (row > image_height - 1) {
        std::clog << "Row index " << row << " is out of bounds.\n";
        return false;
    }
    if (column > image_width - 1) {
        std::clog << "Row index " << row << " is out of bounds.\n";
        return false;
    }

    return true;
}

// Pixel writers.

// Note: row and column index starting from 0.
void bitmap::write_pixel_rgb(int row, int column, uint8_t r, uint8_t g, uint8_t b) {
    if (!this->check_pixel_address_validity(row, column)) {
        return;
    }

    // If I did this correctly
    int pixel_index = 3 * ((row * image_width) + column);
    if (pixel_index > raw_size - 1) {
        std::clog << "Index " << pixel_index << " is out of bounds!\n";
        return;
    }
    pixel_data[pixel_index] = r;
    pixel_data[pixel_index + 1] = g;
    pixel_data[pixel_index + 2] = b;
}

// Like write_pixel_rgb() but using a color vector instead.
void bitmap::write_pixel_vec3(int row, int column, const color &px_color) {
    if (!this->check_pixel_address_validity(row, column)) {
        return;
    }

    int pixel_index = 3 * ((row * image_width) + column);
    if (pixel_index > raw_size - 1) {
        std::clog << "Somehow reaching beyond the actual image?\n";
        return;
    }

    // This is stored in floating-point format, but needs gamma correction.
    // Linear -> Gamma 2
    auto r = linear_to_gamma(px_color.x());
    auto g = linear_to_gamma(px_color.y());
    auto b = linear_to_gamma(px_color.z());

    // We now do a clamped translation from [0.0, 1.0] to [0, 255]
    static const interval intensity(0.000, 0.999);

    uint8_t rbyte = uint8_t(256 * intensity.clamp(r));
    uint8_t gbyte = uint8_t(256 * intensity.clamp(g));
    uint8_t bbyte = uint8_t(256 * intensity.clamp(b));

    pixel_data[pixel_index] = rbyte;
    pixel_data[pixel_index + 1] = gbyte;
    pixel_data[pixel_index + 2] = bbyte;
}

// Bitmap writer/serialization functions

// Writes out bitmap as PPM data (TODO: Implement more formats)
void bitmap::write_as_ppm(std::ostream &out) {
    // PPM header
    out << "P3\n" << image_width << ' ' << image_height << "\n255\n";
    // It should roll over into a new row when the index is a multiple of this.
    int new_row_multiple = 3 * image_width;
    for (int index = 0; index < raw_size; index += 3) {
        // I have to convert to int or it will print as if an ASCII character.
        int r = int(pixel_data[index]);
        int g = int(pixel_data[index + 1]);
        int b = int(pixel_data[index + 2]);

        if (index > 0 && index % new_row_multiple == 0) {
            out << '\n';
        }
        // PPM uses RGB order. Whitespace delineates each color.
        out << r << ' ' << g << ' ' << b << '\n';
    }
}

// Writes out bitmap to BMP, written top-to-bottom order.
void bitmap::write_as_bmp_ttb(std::ostream &out) {
    int new_row_multiple = image_width * 3;
    // To be used as padding with padding_size
    char padding[3] = {0, 0, 0};
    int padding_size = new_row_multiple % 4;
    // It is padded to 4 bytes
    int filled_row_size = new_row_multiple + padding_size;

    int bmp_pxtable_size = filled_row_size * image_height;
    int bmp_file_size = bmp_pxtable_size + sizeof(struct bmp_header);
    struct bmp_header hdr = {.type = 0x4d42, .bmp_size = bmp_file_size,
                             .reserved_1 = 0, .reserved_2 = 0,
                             .pixel_offset = 0x36, // Size of header
                             .hdr_size = 40, // Size of subheader
                             .pixel_width = image_width,
                             // Negative image height causes it to be read top-to-bottom
                             .pixel_height = -image_height,
                             .color_planes = 1, .bpp = 24,
                             .compression_method = BI_RGB,
                             .raw_image_size = bmp_pxtable_size,
                             // px/m, 3780 px/m is approx. 96 px/in
                             .horiz_dpm = 3780, .vert_dpm = 3780,
                             .color_palette_size = 0, // All colors
                             .num_important_colors = 0 // Unimportant
                            };
    // Write header as raw binary data
    out.write(reinterpret_cast<char *>(&hdr), sizeof(hdr));
    for (int index = 0; index < raw_size; index += 3) {
        // Reverse order of pixel colors.
        // (Fix compiler warning about unsigned-signed cast.)
        uint8_t pxbuf[3] = {pixel_data[index + 2],
                            pixel_data[index + 1],
                            pixel_data[index]};
        out.write(reinterpret_cast<char *>(pxbuf), 3);
        if (index > 0 && index % new_row_multiple == 0) {
            out.write(padding, padding_size);
        }
    }
}

// Writes out bitmap to BMP, written bottom-to-top order.
void bitmap::write_as_bmp_btt(std::ostream &out) {
    int new_row_multiple = image_width * 3;
    // To be used as padding with padding_size
    char padding[3] = {0, 0, 0};
    int padding_size = new_row_multiple % 4;
    // It is padded to 4 bytes
    int filled_row_size = new_row_multiple + padding_size;

    int bmp_pxtable_size = filled_row_size * image_height;
    int bmp_file_size = bmp_pxtable_size + sizeof(struct bmp_header);
    struct bmp_header hdr = {.type = 0x4d42, .bmp_size = bmp_file_size,
                             .reserved_1 = 0, .reserved_2 = 0,
                             .pixel_offset = 0x36, // Size of header
                             .hdr_size = 40, // Size of subheader
                             .pixel_width = image_width,
                             // Positive image height causes it to be read bottom-to-top
                             .pixel_height = image_height,
                             .color_planes = 1, .bpp = 24,
                             .compression_method = BI_RGB,
                             .raw_image_size = bmp_pxtable_size,
                             // px/m, 3780 px/m is approx. 96 px/in
                             .horiz_dpm = 3780, .vert_dpm = 3780,
                             .color_palette_size = 0, // All colors
                             .num_important_colors = 0 // Unimportant
                            };
    // Write header as raw binary data
    out.write(reinterpret_cast<char *>(&hdr), sizeof(hdr));

    int top_row = new_row_multiple * (image_height - 1);
    // Future idea: write a full row at a time, buffered beforehand.

    for (int row_index = top_row; row_index > 0; row_index -= new_row_multiple) {
        // We iterate forward through the row, but rows count backwards.
        for (int offset = 0; offset < new_row_multiple; offset += 3) {
            int index = row_index + offset;
            // Reverse order of pixel colors
            uint8_t pxbuf[3] = {pixel_data[index + 2],
                                pixel_data[index + 1],
                                pixel_data[index]};

            out.write(reinterpret_cast<char *>(pxbuf), 3);
        }
        // Since row is finished, write padding
        out.write(padding, padding_size);
    }
}
