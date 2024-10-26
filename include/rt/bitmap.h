#pragma once

#include "color.h"
#include <cstdint>
#include <iostream>
#include <cstdio>
// For std::unique_ptr<>
#include <memory>

namespace rt {
// Stores a simple RGB color in a convenient structure.
struct rgb {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

// To allow querying supported types at runtime. May be queried like rt::BitmapOutput::PNG.
// Note: This cannot be implicitly converted to an int, it must have explicit static_cast<int>().
enum class BitmapOutput { PPM, BMP, PNG, JPEG };

/* Class for a RGB24 bitmap (R8G8B8).
 * Includes functions to serialize as a few different formats.
 * Note that the instantiated bitmap class must outlive any pointers to
 * pixel_data or it will be use-after-free.
 *
 * Thread-Safety: It is thread-safe to write to non-overlapping portions of
 * this image from different threads, but it is *not* thread-safe to modify
 * the image while writing to a file (because there is no guarantee of whether
 * the original or modified pixel is read by the writer).
 */
class bitmap {
  public:
    // Goes from left-to-right, top-to-bottom, in RGB order (8 bpc, 24bpp)
    std::unique_ptr<uint8_t[]> pixel_data;

    bitmap(int image_width, int image_height) {
        this->image_width = image_width;
        this->image_height = image_height;
        // 3 bytes for a 24bpp pixel
        this->raw_size = image_width * image_height * 3;
        // I use a unique_ptr<> because I need sane move semantics.
        pixel_data = std::make_unique<uint8_t[]>(this->raw_size);
    }

    int get_image_width() {
        return image_width;
    }

    int get_image_height() {
        return image_height;
    }

    // Row and column index starting from 0.
    void write_pixel_rgb(int row, int column, uint8_t r, uint8_t g, uint8_t b);

    // Like above but using a unified struct rgb.
    void write_pixel_rgb(int row, int column, struct rgb rgb) {
        write_pixel_rgb(row, column, rgb.r, rgb.g, rgb.b);
    }

    // Like write_pixel_rgb() but using a color vector instead.
    void write_pixel_vec3(int row, int column, const color &px_color);

    // Writes out bitmap as PPM data
    void write_as_ppm(std::ostream &out);

    // Writes out bitmap to BMP, written top-to-bottom order.
    // This function is slower because it is written pixel-by-pixel.
    void write_as_bmp_ttb(std::ostream &out);

    // Writes out bitmap to BMP, written bottom-to-top (standard) order.
    // Note that this function is faster because it is line-buffered.
    void write_as_bmp_btt(std::ostream &out);

    // Writes out bitmap to a PNG (with default settings).
    void write_as_png(std::ostream &out);

    // Writes out bitmap to a JPEG (for settings, see bitmap.c++).
    void write_as_jpeg(std::ostream &out);

    // Write to file (selected based on type specified)
    void write_to_file(std::ostream &out, BitmapOutput filetype);

    // Returns whether a type is supported.
    static bool type_is_supported(BitmapOutput filetype);

  private:
    // Internal and immutable data here.
    int image_width;
    int image_height;
    int raw_size;

    /* Internal methods here. Internal methods are called by this->method(),
     * or simply by method() (no reference to object).
     */
    bool check_pixel_address_validity(int row, int column);
};

}
