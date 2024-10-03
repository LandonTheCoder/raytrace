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

// Determine what formats are supported.
#include "config.h"

// Optional formats included here.
#ifdef ENABLE_PNG
#include <png.h>
// Needed for setjmp()/longjmp(), used for error handling.
#include <csetjmp>
#endif

// I haven't implemented support for advanced formats yet. I hope to do PNG first.

// Global functions

bool bitmap::type_is_supported(BitmapOutputType filetype) {
    switch (filetype) {
      case BMPOUT_PPM:
        return true;
        break;
      case BMPOUT_BMP:
        return true;
        break;
      // Insert optional types here. If supported, the code to return true is
      // included. Otherwise, it isn't (falling back to false).
#ifdef ENABLE_PNG
      case BMPOUT_PNG:
        return true;
        break;
#endif
      default:
        return false;
        break;
    }
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

// Writes out a PNG (support is optional)
#ifdef ENABLE_PNG

static void png_write_callback(png_structp png_ptr, png_bytep data, png_size_t len) {
    std::ostream *out_ptr = (std::ostream *)png_get_io_ptr(png_ptr);
    std::ostream &out = *out_ptr;

    out.write(reinterpret_cast<char *>(data), len);
}

static void png_flush_callback(png_structp png_ptr) {
    std::ostream *out_ptr = (std::ostream *)png_get_io_ptr(png_ptr);
    (*out_ptr) << std::flush;
}

void bitmap::write_as_png(std::ostream &out) {
    // Implement the real thing here.
    // Determine which args are necessary.
    // With typedef void (*)(png_structp, png_const_charp) err_fn_t:
    // png_create_write_struct(char *verstring, void *err_ptr, err_fn_t err_fn, err_fn_t warn_fn)
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                                  nullptr,
                                                  nullptr,
                                                  nullptr);
    if (!png_ptr) {
        std::clog << "Failed to initialize libpng writer\n";
        return;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, png_infopp(NULL));
        std::clog << "Failed to initialize libpng info structure\n";
        return;
    }

    // They make me use setjmp/longjmp() for error handling :(
    if (setjmp(png_jmpbuf(png_ptr))) {
        // Error occurred
        png_destroy_write_struct(&png_ptr, &info_ptr);
        std::clog << "Something happened while writing the PNG file.\n";
        return;
    }

    png_set_write_fn(png_ptr, png_voidp(&out), png_write_callback, png_flush_callback);

    png_set_IHDR(png_ptr,
                 info_ptr,
                 image_width,
                 image_height,
                 8, // Bits per channel
                 PNG_COLOR_TYPE_RGB, // 8bpc means 24bpp
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    // Start here?
    // Write header data
    png_write_info(png_ptr, info_ptr);
    // It takes different "row pointers". png_bytep is unsigned char *
    png_bytep row_pointers[image_height];

    if (image_height > PNG_UINT_32_MAX / sizeof(png_bytep))
        png_error(png_ptr, "Image is too tall to process in memory");

    for (int index = 0; index < image_height; index++) {
        row_pointers[index] = pixel_data + index * image_width * 3;
    }

    png_write_image(png_ptr, row_pointers);

    png_write_end(png_ptr, info_ptr);

    png_destroy_write_struct(&png_ptr, &info_ptr);
}
#else
void bitmap::write_as_png(std::ostream &out) {
    return;
}
#endif
