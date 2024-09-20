# Ray Tracing in One Weekend #
## Progress ##
 - I am at section 9

## Mathematical Notes ##

### Sphere equation ###
x^2 + y^2 + z^2 = r^2 for a sphere centered upon the origin, where (x, y, z) is a point on its surface. Accordingly, for a sphere with a center at point (Cx, Cy, Cz), r^2 = (Cx - x)^2 + (Cy - y)^2 + (Cz - z)^2.

Vector from point P to center C is (C - P). (C - P) · (C - P) = (Cx - x)^2 + (Cy - y)^2 + (Cz - z)^2. Accordingly, r^2 = (C - P) · (C - P). Any point satisfying that equation is on the sphere.

If a ray P(t) = Q + t \* d hits the sphere, there is a value of t satisfying the sphere equation given above. That means we need to look for (C - P(t)) · (C - P(t)) = r^2, or (C - (Q + t \* d)) · (C - (Q + t \* d)) = r^2. We can separate terms based on presence of a t: (-t \* d + (C - Q)) · (-t \* d + (C - Q)) = r^2. Then, we can distribute the dot product: t^2 \* d · d - 2t \* d · (C - Q) + (C - Q) · (C - Q) - r^2 = 0. The t can be solved for by quadratic equation, which gives that a = d · d, b = -2d · (C - Q), c = (C - Q) · (C - Q) - r^2. While algebraically its square root can be positive (2 real solutions), negative (no real solutions), or 0 (1 real solution), it relates to the geometry (0 roots is no intersection, 1 root is 1 intersection, 2 roots is 2 intersections).

## Programming Notes ##
std::shared\_ptr\<T\> stores an automatically refcounted pointer, and safely (hopefully?) deletes it once refcount hits 0. A shared\_ptr is initialized by assigning to it with make\_shared<T>(arguments).

std::vector\<T\> stores a collection of a specified type in a list that grows automatically.

## Structure of BMP file ##
 - Bitmap file header: 14 bytes
   - addr 0x00: i16 type = {'B', 'M'}
   - addr 0x02: i32 bmp\_size (size of file in bytes)
   - addr 0x06: i16 reserved\_1 = 0
   - addr 0x08: i16 reserved\_2 = 0
   - addr 0x0A: i32 pixel_offset = 0x36 (offset where pixel data begins)
 - DIB Header: for BITMAPINFOHEADER, 40 bytes
   - addr 0x0E: i32 hdr\_size = 40
   - addr 0x12: i32 pixel\_width
   - addr 0x16: i32 pixel\_height
   - addr 0x1A: i16 color\_planes = 1
   - addr 0x1C: i16 bpp = (24 or 32)
   - addr 0x1E: i32 compression\_method
     - BI_RGB = 0 (uncompressed)
     - BI_JPEG = 4 (JPEG)
     - BI_PNG = 5 (PNG)
   - addr 0x22: i32 raw\_image\_size = 0 (size of uncompressed data)
   - addr 0x26: i32 horiz_dpm (horizontal px/m, signed)
   - addr 0x2A: i32 vert_dpm (vertical px/m, signed)
   - addr 0x2E: i32 color\_palette\_size = 0 (number of colors in palette, 0 is 2^n by default)
   - addr 0x32: i32 num\_important\_colors = 0
 - Color table?
   - Might contain 2^bitdepth colors, stored in order {blue, green, red, reserved = 0}
   - May be omitted for 16+ bpp?
 - Padding?
 - Pixel table (each row is padded to a multiple of 4 bytes)
   - row\_size = ceil(bpp * pixel\_width / 32) * 4
   - Images are stored starting from bottom-left, left to right, bottom-to-top, unless pixel\_height is negative.
   - 24bpp is stored as BGR format instead of RGB.

## Structure of PPM text file ##
 - Header: printf("P3\n%i %i\n255\n", width, height)
 - Pixel format: red, green, blue (in that order, printed as decimal text like "128") separated by whitespace, after the blue color it rolls over to the next pixel.

## Structure of PPM binary file ##
 - Header: printf("P3\n%i %i\n255\n", width, height)
 - Pixel format: A pixel table in R8G8B8 format, with no padding.

## Structure of PNG file ##
 - Header: "\x89PNG\r\n\x1a\n"
 - A series of chunks in the following format (this is big-endian):
   - i32 length
   - i32 chunk\_type (this is ASCII text, with case or bit 5 of a character used as a bitfield)
     - First letter: Critical (on/uppercase if true)
     - Second letter: Public (standardized) chunk (on/uppercase if true)
     - Third letter: Reserved (always on/uppercase, treat as unrecognized if off/lowercase)
     - Fourth letter: Can be copied safely by editors without support for that chunk (on/uppercase if true)
   - u8 data\[length\]
   - i32 crc32

 - The first chunk must be an "IHDR" chunk

### IHDR Chunk ###
 - u32 width (in pixels)
 - u32 length
 - u8 bits\_per\_sample (can be 1/2/4/8/16)
   - For color type 0: 1/2/4/8/16
   - For color type 2: 8/16 (Note: each channel is a sample, so 24bpp is value 8)
   - For color type 3: 1/2/4/8
   - For color type 4: 8/16
   - For color type 6: 8/16
 - u8 color\_type (can be 0/2/3/4/6, acts as bitmask)
   - 0: Greyscale
   - 1: (Bitmask) indicates palette used
   - 2: (Bitmask) indicates truecolor, (Value) 24bpp RGB
   - 4: (Bitmask) indicates transparent, (Value) Greyscale with alpha
   - 6: Truecolor with transparency, RGBA (32bpp)
 - u8 compression\_method = 0
 - u8 filter\_method = 0
 - u8 interlace\_method (0 uninterlaced, 7 Adam7 interlace)

### PLTE Chunk ###
Contains palette for indexed color.

### IDAT Chunk ###
Contains image. Image is compressed through 2 steps: a "filter" step, then DEFLATE.

### IEND Chunk ###
Marks end of image.
