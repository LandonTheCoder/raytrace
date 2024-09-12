#pragma once
#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif
#define BI_RGB 0
#define BI_JPEG 4
#define BI_PNG 5

// Needs byte-level alignment or it will be 2 bytes too long.
#pragma pack(push)
#pragma pack(1)

// Note: integers are little-endian in the format.
struct bmp_header {
    // Should be equal to {'B', 'M'}, 0x4d42 little-endian or 0x424d big-endian
    int16_t type; // Note: This would be misaligned if not using byte-level packing.
    // Size of whole file in bytes (header + pixel table)
    int32_t bmp_size;
    // Should be equal to 0
    int16_t reserved_1;
    int16_t reserved_2;
    // Offset in file where pixel data begins
    int32_t pixel_offset;
    // Here begins the BITMAPINFOHEADER.
    int32_t hdr_size; // Should be 40
    int32_t pixel_width;
    /* By default bmp is read from bottom-to-top, left-to-right. Negating the
     * height causes it to be read top-to-bottom, left-to-right.
     */
    int32_t pixel_height;
    int16_t color_planes; // Always 1
    int16_t bpp; // I should use 24 (which is BGR format, padded)
    int32_t compression_method; // BI_RGB is uncompressed
    int32_t raw_image_size; // Can be set to 0 when uncompressed
    int32_t horiz_dpm; // horizontal px/m, 96dpi is 3779.5276 m
    int32_t vert_dpm; // vertical px/m, should match horiz_dpm
    int32_t color_palette_size; // Set equal to 0 to enable all colors
    int32_t num_important_colors; // Set equal to 0
    /* Note: The pixel table has each pixel row padded to a multiple of 4
     * bytes. The padding should be zero-filled. */
};

#pragma pack(pop)
