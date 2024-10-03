#pragma once

#include "color.h"
#include <cstdint>
#include <iostream>
#include <cstdio>

// To allow querying supported types at runtime. BMPOUT_TERMINATOR is to
// determine the end of a supported_type_list.
enum BitmapOutputType { BMPOUT_TERMINATOR, BMPOUT_PPM, BMPOUT_BMP, BMPOUT_PNG, BMPOUT_JPEG };

/* Class for a RGB24 bitmap (R8G8B8).
 * Includes functions to serialize as a few different formats.
 * Note that the instantiated bitmap class must outlive any pointers to
 * pixel_data or it will be use-after-free.
 */
class bitmap {
  public:
    // Goes from left-to-right, top-to-bottom, in RGB order (8 bpc, 24bpp)
    uint8_t *pixel_data;

    bitmap(int image_width, int image_height) {
        this->image_width = image_width;
        this->image_height = image_height;
        // 3 bytes for a 24bpp pixel
        this->raw_size = image_width * image_height * 3;
        // Is there a safer way to do this?
        pixel_data = new uint8_t [this->raw_size];
    }
    ~bitmap() {
        delete pixel_data;
    }

    int get_image_width() {
        return image_width;
    }

    int get_image_height() {
        return image_height;
    }

    // Row and column index starting from 0.
    void write_pixel_rgb(int row, int column, uint8_t r, uint8_t g, uint8_t b);

    // Like write_pixel_rgb() but using a color vector instead.
    void write_pixel_vec3(int row, int column, const color &px_color);

    // Writes out bitmap as PPM data (TODO: Implement more formats)
    void write_as_ppm(std::ostream &out);

    // Writes out bitmap to BMP, written top-to-bottom order.
    void write_as_bmp_ttb(std::ostream &out);

    // Writes out bitmap to BMP, written bottom-to-top (standard) order.
    void write_as_bmp_btt(std::ostream &out);

    // Returns whether a type is supported.
    static bool type_is_supported(BitmapOutputType filetype);

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
