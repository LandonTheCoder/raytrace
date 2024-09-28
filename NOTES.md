# Ray Tracing in One Weekend #
## Progress ##
 - I am at section 11.3

## Mathematical Notes ##

### Sphere equation ###
x^2 + y^2 + z^2 = r^2 for a sphere centered upon the origin, where (x, y, z) is a point on its surface. Accordingly, for a sphere with a center at point (Cx, Cy, Cz), r^2 = (Cx - x)^2 + (Cy - y)^2 + (Cz - z)^2.

Vector from point P to center C is (C - P). (C - P) · (C - P) = (Cx - x)^2 + (Cy - y)^2 + (Cz - z)^2. Accordingly, r^2 = (C - P) · (C - P). Any point satisfying that equation is on the sphere.

If a ray P(t) = Q + t \* d hits the sphere, there is a value of t satisfying the sphere equation given above. That means we need to look for (C - P(t)) · (C - P(t)) = r^2, or (C - (Q + t \* d)) · (C - (Q + t \* d)) = r^2. We can separate terms based on presence of a t: (-t \* d + (C - Q)) · (-t \* d + (C - Q)) = r^2. Then, we can distribute the dot product: t^2 \* d · d - 2t \* d · (C - Q) + (C - Q) · (C - Q) - r^2 = 0. The t can be solved for by quadratic equation, which gives that a = d · d, b = -2d · (C - Q), c = (C - Q) · (C - Q) - r^2. While algebraically its square root can be positive (2 real solutions), negative (no real solutions), or 0 (1 real solution), it relates to the geometry (0 roots is no intersection, 1 root is 1 intersection, 2 roots is 2 intersections).

### Lambertian distribution ###
Lambertian distribution is a more accurate way to implement diffuse surfaces. It scatters reflected rays in a manner proportional to cos(ϕ), with ϕ as the angle between reflected ray and surface normal. In other words, a ray is most likely to scatter in a direction near surface normal, and less in directions away from it. It can be created by adding a random unit vector to the normal vector. With hit point as p and normal vector as n, there are 2 sides at that intersection, so there are 2 tangent unit spheres displaced from surface by length of radius (1 for unit sphere). 1 is displaced in direction of normal (n) and 1 is displaced towards -n. There are spheres touching surface centered at (P + n) and (P - n), where (P - n) is inside surface and (P + n) is outside. We pick a point S on the unit sphere on the outside, and send a ray from P to S (vector (S - P)).

### Gamma Correction ###
Gamma correction is a transform applied before writing a pixel value. A linear space means an image without any gamma correction, and gamma space is one with that correction applied. In our case, the lack of that correction causes it to be irrealistically dark.

We are applying a "gamma 2" correction, meaning a correction to exponent 1/gamma (in this case 1/2 or square root) for linear to gamma. (It would be to exponent gamma or squaring to go from gamma to linear.) That results in a more consistent change of color.

## Surface/Materials Notes ##

### Diffuse Surfaces ###
The addition of objects and multiple rays per pixel (which is done in a square which comes \[-.5, .5\] out from the pixel itself) allows making diffuse (matte) surfaces. We can make the material and geometry tightly bound, or separated (we pick separated for flexibility).

Diffuse objects which don't emit their own light will take on the color of their surroundings, but it does take influence from the "intrinsic color" of the material surface. Light which reflects off has a randomized direction. (If 3 rays are sent into a crack in a diffuse surface, they will have different random behavior.) Light may also be absorbed instead of reflected, and as the surface gets darker, the likelihood of absorption gets higher. There needs to be a means to make sure that a random vector only gives results on the surface of a hemisphere, and the simplest is to reject invalid ones. It amounts to: Generate random vector inside unit sphere, normalize to sphere surface, invert normalized vector if in wrong hemisphere. If a ray bounces off a material and keeps 100% of color, it is white. If it bounces off and keeps 0% of color, it is black. Note that floating-point rounding errors can cause rays to be sent which aren't flush with the surface, and can erroneously intersect with it, causing color distortion. This problem is called "shadow acne".

### Metal Surfaces ###
Metals, of course, reflect differently. We can randomize the direction of reflection by using a small sphere and choosing a new endpoint for the ray. We use a random point from surface of a sphere centered on original endpoint, which is scaled by "fuzz factor". As the "fuzz sphere" gets bigger, the reflections get fuzzier. (We can simply add a fuzziness parameter corresponding to radius of fuzz sphere, with 0 meaning no distortion.) However, for big spheres it could scatter below the surface (in which case it should be absorbed). It also has to be scaled consistently compared to reflection vector (which can arbitrarily vary in length), which means normalizing reflected ray.

### Dielectric Surfaces ###
Clear materials (like water, glass, diamond) are dielectrics. When a light ray hits them, it splits into a reflected ray and a refracted ray. We handle it by randomly choosing which we perform, and only scatter 1 ray per interaction. A reflected ray hits and bounces off, while a refracted ray bends once it enters the material. How much it bends is determined by the refractive index (which is a number, where higher values refract more). For a transparent object within another transparent object, use relative refractive index (inner index/outer index).

Refraction is described by Snell's Law: η · sin(Θ) = η' · sin(Θ'), where η (eta, U+3B7) is refractive index and Θ is angle from normal. To find direction of refracted ray, solve for sin(Θ'), so sin(Θ') = (η/η') · sin(Θ). On refracted side of surface, there is a refracted ray R', a normal n', and an angle Θ' between them. We can split R' into parts perp. to n' and parallel to n'. We find that R' perpendicular is (η/η')(R + cos(Θ) * n), and that R' parallel is -sqrt(1 - |R' perp.|^2 * n). Everything except cos(Θ) is known, and dot product is magnitude of 2 vectors times the angle between them, so if the 2 vectors are unit vectors, the dot product is cos(Θ). Now, R' perp. is (η/η')(R + (-R · n)*n).

There can be ray angles for which refraction doesn't work and it must be reflected. As an example, with ray inside glass with air on outside (η = 1.5 and η' = 1.0), that can potentially result in a broken equation because it would make sin(Θ') greater than 1. In that scenario, it must reflect. In the scenario where all light is reflected, it is called total internal reflection, and is why water-air boundary can act as a perfect mirror when underwater sometimes. We reflect if ri * sin\_theta \> 1.0, and can solve for sin\_theta by sin(Θ) = sqrt(1 - cos(Θ)^2) and cos(Θ) = R · n.

## Material Types ##
We are using an abstract class for materials to implement flexibility more easily. It needs to support producing a scattered ray, and say how much to attenuate it (if scattered).

## Programming Notes ##
std::shared\_ptr\<T\> stores an automatically refcounted pointer, and safely (hopefully?) deletes it once refcount hits 0. A shared\_ptr is initialized by assigning to it with make\_shared<T>(arguments).

std::vector\<T\> stores a collection of a specified type in a list that grows automatically.

## Ideas for Expansion ##
 - Of course, refactor to better control output.
 - Instead of writing a bunch of spaces, use ANSI escape sequences to clear the line.
   - "\x0D" or "\r" is CR, goes to beginning of line
   - "\x1B" or "\e" or "\033" is Escape character
   - "\x1B[0K" (or "\033[0K") clears the current line (from cursor to end)
   - "\x1B[1K" (or "\033[1K") clears the current line (from beginning to cursor)
   - "\x1B[2K" (or "\033[2K") clears line (from beginning to end)
   - Enabling on Windows: See [Microsoft Documentation](https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences)
   - The line-clearing "Done" string can be: "\r\033[0KDone.\n"
 - Make sure stdout works correctly on Windows (may require enabling binary mode). See [_setmode()](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/setmode)

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
