# Raytrace in a Weekend #
See URL [Ray Tracing in One Weekend Series](https://raytracing.github.io), and book [Ray Tracing in a Weelend](https://raytracing.github.io/books/RayTracingInOneWeekend.html).

The basic gist of what I'm doing is:
1. Calculate the ray from the “eye” through the pixel,
2. Determine which objects the ray intersects, and
3. Compute a color for the closest intersection point.

This is based on the Ray Tracing in One Weekend tutorial, with a few modifications added for flexibility (such as modifying it so that it can output to formats other than PPM, with room for expansion).

## Build Options ##
I have 1 option: the "png" feature, which enables/disables libpng support (to enable writing PNG files). I will use options to allow picking optional features (like image-format support).

I highly suggest you supply a buildtype option or it will be painfully slow. By default, it uses "--buildtype debug" which provides no optimization options at all. I highly suggest using either the debugoptimized buildtype (equivalent to -O2 -g), the release buildtype (equivalent to -O3), or maybe minsize if you are space/cache-constrained (equivalent to -Os -g).

Even the debugoptimized buildtype reduced image generation time from 51 seconds to 12.7 seconds (as of Ch. 13.2). The release buildtype brought it down to 12.5 seconds. Minsize resulted in 14.4 seconds on my system, so it may not make as much sense for you (because it won't do some things that can increase size like loop unrolling).

### Speed Test: Final version ###
The file has 500 samples per pixel, at 1200×675 px, and outputs in PPM format.

 - plain (No options): 15 hours 24 minutes 6.34 seconds
 - debug (-O0 -g): 15 hours 24 minutes 6.32 seconds
 - debugoptimized (-O2 -g): 2 hours 9 minutes 35.94 seconds
 - release (-O3): 2 hours 9 minutes 6.97 seconds
 - minsize (-Os -g): 2 hours 14 minutes 6.38 seconds

## Enhancements ##
I have enhanced it so that it can take arguments specifying an output file, supporting PPM, BMP, and (optionally) PNG. I hope to support JPEG in the future.

I have some quirks to help support Windows, but it isn't tested on Windows.
