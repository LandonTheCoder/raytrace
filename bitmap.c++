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

/* I need to emulate VLAs on compilers lacking the extension to support it.
 * Note that VLAs are standard in C (optionally in C11+), but not C++.
 * GCC/Clang supports it (standard) in C, and (extension) in C++.
 * A popular vendor compiler for Windows supports it in neither C nor C++.
 * That means I have to emulate it using alloca() (which allocates to stack).
 * I don't want to do a heap allocation because it means overhead (and because
 * unique_ptr<> for a temporary variable is irritating).
 * I also have the VLA form because it is more readable than the alloca() or
 * malloc() equivalent of either (and I would have to look up how to do it with
 * the new operator).
 *
 * Alternative implementation: unique_ptr<T[]> x = make_unique<T[]>(image_height);
 */
#ifdef __GNUC__
// To note a compiler (independent of vendor) which allows VLAs
#define HAVE_VLA
#else
// GNU C extensions not supported (which includes VLAs in C++)
// This means VLA emulation is required.
#include <arch/alloca.h>
#endif

// Quality definitions
#define JPEG_QUALITY 95

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
#if defined ENABLE_LIBJPEG || defined ENABLE_TJPEG3
      case BMPOUT_JPEG:
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

void bitmap::write_to_file(std::ostream &out, BitmapOutputType filetype) {
    // Note: I expect you have already checked to make sure you aren't trying
    // to use an unsupported writer.
    switch (filetype) {
      case BMPOUT_PPM:
        write_as_ppm(out);
        break;
      case BMPOUT_BMP:
        write_as_bmp_btt(out);
        break;
      case BMPOUT_PNG:
        write_as_png(out);
        break;
      case BMPOUT_JPEG:
        write_as_jpeg(out);
        break;
      default:
        std::clog << "Unsupported file type!\n";
        break;
    }
}

// Writes out bitmap as PPM data
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

// Implementations of optional writers.

#ifdef ENABLE_PNG
#include <png.h>
// Needed for setjmp()/longjmp(), used for error handling.
#include <csetjmp>

// Callbacks for PNG writer to write to an ostream
static void png_write_callback(png_structp png_ptr, png_bytep data, png_size_t len) {
    std::ostream *out_ptr = (std::ostream *)png_get_io_ptr(png_ptr);
    std::ostream &out = *out_ptr;

    out.write(reinterpret_cast<char *>(data), len);
}

static void png_flush_callback(png_structp png_ptr) {
    std::ostream *out_ptr = (std::ostream *)png_get_io_ptr(png_ptr);
    (*out_ptr) << std::flush;
}

// Writes out a PNG (support is optional)
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
    // This returns 0 initially, something else if jumped back to.
    // Note that setjmp() must be used every time a new function accesses libpng.
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

    // Set pixel density (in px/m)
    png_set_pHYs(png_ptr, info_ptr, 3780, 3780, PNG_RESOLUTION_METER);

    // Start here?
    // Write header data
    png_write_info(png_ptr, info_ptr);
    // It takes different "row pointers". png_bytep is unsigned char *
#ifdef HAVE_VLA
    png_bytep row_pointers[image_height];
#else
    // Fallback based on alloca().
    png_bytepp row_pointers = STACK_VLARRAY(png_bytep, image_height);
#endif

    if (uint32_t(image_height) > PNG_UINT_32_MAX / sizeof(png_bytep))
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

#ifdef ENABLE_TJPEG3
// I use the TurboJPEG API, not the libjpeg API.
#include <turbojpeg.h>

// Write out a JPEG (using TurboJPEG 3). Note that TurboJPEG is currently untested.
void bitmap::write_as_jpeg(std::ostream &out) {
    tjhandle j_handle = tj3Init(TJINIT_COMPRESS); // Returns handle or NULL if error

    if (!j_handle) {
        std::clog << "Failed to initialize libjpeg-turbo writer: "
                  << tj3GetErrorStr(j_handle) << '\n';
        return;
    }

    // Quality level and subsampling *must* be set before attempting compression.
    // Quality level 80
    int val_check = tj3Set(j_handle, TJPARAM_QUALITY, JPEG_QUALITY);
    // 4:2:0 chroma subsampling as a default (we can do 4:4:4, 4:2:2, 4:2:0)
    val_check = tj3Set(j_handle, TJPARAM_SUBSAMP, TJSAMP_420);

    // I thought it would be nice for them to know the DPI.
    val_check = tj3Set(j_handle, TJPARAM_DENSITYUNITS, 1); // DPI
    val_check = tj3Set(j_handle, TJPARAM_XDENSITY, 96);
    val_check = tj3Set(j_handle, TJPARAM_YDENSITY, 96);

    uint8_t *jpeg_buf = nullptr; // jpeg-turbo will auto-allocate the buffer
    size_t jpeg_buf_size;

    // This is the important one.
    val_check = tj3Compress8(j_handle,
                             pixel_data, // Note: it expects a linear top-to-bottom framebuffer here.
                             image_width,
                             0, // The "pitch", which is autoderived here to 3 * image_width
                             image_height,
                             TJPF_RGB, // R8G8B8
                             &jpeg_buf,
                             &jpeg_buf_size);

    if (val_check == -1) {
        std::clog << "Failed to compress JPEG: " << tj3GetErrorStr(j_handle) << '\n';
        return;
    }

    // We should have a working JPEG buffer now.
    out.write(reinterpret_cast<char *>(jpeg_buf), jpeg_buf_size);

    tj3Free(jpeg_buf); // They require us to use their free function for auto-allocated buffers.
}
#elif defined ENABLE_LIBJPEG
// Fallback: libjpeg-turbo
// Must be included before libjpeg headers
#include <cstdio>
// For free()
#include <cstddef>
#include <jpeglib.h>
#include <jerror.h>

// Write out a JPEG (using libjpeg/libjpeg-turbo)
void bitmap::write_as_jpeg(std::ostream &out) {
    struct jpeg_compress_struct j_comp;
    struct jpeg_error_mgr jerr;

    uint8_t *jpeg_buf = nullptr;
    unsigned long jpeg_buf_size = 0;
#ifdef HAVE_VLA
    JSAMPROW row_pointers[image_height];
#else
    // I prefer the VLA form because it is more readable, and better tested.
    JSAMPARRAY row_pointers = STACK_VLARRAY(JSAMPROW, image_height);
#endif

    j_comp.err = jpeg_std_error(&jerr); // Initialize error handling
    jpeg_create_compress(&j_comp);

    jpeg_mem_dest(&j_comp, &jpeg_buf, &jpeg_buf_size);

    j_comp.image_width = image_width;
    j_comp.image_height = image_height;
    j_comp.input_components = 3; // RGB
    j_comp.in_color_space = JCS_RGB;
    j_comp.data_precision = 8; // bpc

    jpeg_set_defaults(&j_comp);

    jpeg_set_quality(&j_comp, JPEG_QUALITY, true); // last arg limits to baseline JPEG

    /* Chroma subsampling works like this in the libjpeg API:
     * There is a j_comp.comp_info[i].h_samp_factor and a
     * j_comp.comp_info[i].v_samp_factor. Value must be between [1, 4].
     * Higher h/v_samp_factor numbers indicate higher resolution.
     * Their relation to each other determines quality.
     * j_comp.comp_info[0] is Y (Luminance)
     * j_comp.comp_info[1] is Cb (U)
     * j_comp.comp_info[2] is Cr (V)
     * By default, Y is 2h 2v, and U/V is 1h 1v (meaning Y has twice as much
     * resolution as chrominance). That amounts to 4:2:0 chroma subsampling.
     * The types of subsampling:
     * 4:4:4 is no chroma subsampling (chroma and luma are full resolution).
     * 4:2:2 is a reduction by factor of 2 horizontally (2×1 blocks).
     * 4:2:0 is a reduction by factor of 2 in both directions (2×2 blocks).
     */

    // To set 4:4:4 subsampling:
    //j_comp.comp_info[0].h_samp_factor = j_comp.comp_info[0].v_samp_factor = 1;

    jpeg_start_compress(&j_comp, true); // True ensures a full JPEG

    for (int index = 0; index < image_height; index++) {
        row_pointers[index] = pixel_data + index * image_width * 3;
    }

    int check = jpeg_write_scanlines(&j_comp, row_pointers, image_height);

    int num_tries = 0;
    while (check < image_height) {
        // It should have written everything, but try again.
        std::clog << "libjpeg was supposed to write " << image_height
                  << " lines, actually wrote " << check << " lines.\n";
        check = jpeg_write_scanlines(&j_comp, row_pointers + check, image_height - check);
        num_tries++;
        if (num_tries == 10) {
            std::clog << "Tried this 10 times and failed, now giving up.\n";
            return;
        }
    }
    jpeg_finish_compress(&j_comp);
    out.write(reinterpret_cast<char *>(jpeg_buf), jpeg_buf_size);

    jpeg_destroy_compress(&j_comp);

    // Source code seems to say I need to free() jpeg_buf myself.
    free(jpeg_buf);
}
#else
void bitmap::write_as_jpeg(std::ostream &out) {
    return;
}
#endif
