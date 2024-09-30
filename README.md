# Raytrace in a Weekend #
See URL [Ray Tracing in One Weekend Series](https://raytracing.github.io), and book [Ray Tracing in a Weelend](https://raytracing.github.io/books/RayTracingInOneWeekend.html).

The basic gist of what I'm doing is:
1. Calculate the ray from the “eye” through the pixel,
2. Determine which objects the ray intersects, and
3. Compute a color for the closest intersection point.

This is based on the Ray Tracing in One Weekend tutorial, with a few modifications added for flexibility (such as modifying it so that it can output to formats other than PPM, with room for expansion).

## Build Options ##
No unique options currently. I highly suggest you supply a buildtype option or it will be painfully slow. By default, it uses "--buildtype debug" which provides no optimization options at all. I highly suggest using either the debugoptimized buildtype (equivalent to -O2 -g), the release buildtype (equivalent to -O3), or maybe minsize if you are space/cache-constrained (equivalent to -Os -g).

Even the debugoptimized buildtype reduced image generation time from 51 seconds to 12.7 seconds (as of Ch. 13.2). The release buildtype brought it down to 12.5 seconds. Minsize resulted in 14.4 seconds on my system, so it may not make as much sense for you (because it won't do some things that can increase size like loop unrolling).

### Speed Test: Final version ###
The file has 500 samples per pixel, at 1200×675 px.

 - plain (No options): (Untested currently)
 - debug (-O0 -g): (Untested currently)
 - debugoptimized (-O2 -g): 2 hours 9 minutes 35.94 seconds
 - release (-O3): 2 hours 9 minutes 6.97 seconds
 - minsize (-Os -g): 2 hours 14 minutes 6.38 seconds

## Enhancements ##
Currently, my sole enhancement is the framework to be able to support more image formats in the future (which isn't utilized). I plan to reorganize the code slightly such that main() can determine which output format is used. I will also allow making the program output to a file specified on the command line, and picking file types. In the immediate term, I will implement PPM and BMP (BMP is fairly easy to do), and I plan to implement PNG and JPEG using external libraries. If no arguments are provided, default to printing a PPM image to stdout (which is safe on all platforms).
