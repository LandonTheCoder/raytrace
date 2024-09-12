#pragma once

#include "vec3.h"
#include <cstdint>
#include <iostream>
#include <cstdio>

using color = vec3;

/* Class for a RGB24 bitmap (R8G8B8).
 * Includes functions to serialize as a few different formats.
 * Note that the instantiated bitmap class must outlive any pointers to
 * pixel_data or it will be use-after-free.
 */
class bitmap {
  public:
    int image_width;
    int image_height;
    int raw_size;
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

    // Row and column index starting from 0.
    void write_pixel_rgb(int row, int column, uint8_t r, uint8_t g, uint8_t b);

    // Like write_pixel_rgb() but using a color vector instead.
    void write_pixel_vec3(int row, int column, const color &px_color);

    // Writes out bitmap as PPM data (TODO: Implement more formats)
    void write_as_ppm(std::ostream &out);

    // Writes out bitmap to BMP, written top-to-bottom order.
    void write_as_bmp_ttb(std::ostream &out);

  private:
    /* Internal methods here. Internal methods are called by this->method(),
     * or simply by method() (no reference to object).
     */
    bool check_pixel_address_validity(int row, int column);
};

void write_color(std::ostream &out, const color &pixel_color);
