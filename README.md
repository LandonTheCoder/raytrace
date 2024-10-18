# Raytrace in a Weekend #
See URL [Ray Tracing in One Weekend Series](https://raytracing.github.io), and book [Ray Tracing in a Weelend](https://raytracing.github.io/books/RayTracingInOneWeekend.html).

The basic gist of what I'm doing is:
1. Calculate the ray from the “eye” through the pixel,
2. Determine which objects the ray intersects, and
3. Compute a color for the closest intersection point.

This is based on the Ray Tracing in One Weekend tutorial, with a few modifications added for flexibility (such as modifying it so that it can output to formats other than PPM, with room for expansion).

## Build Options ##
I have 2 options: the "png" feature, which enables/disables libpng support (to enable writing PNG files), and the "jpeg" feature, which enables/disables libjpeg/libjpeg-turbo support (which enables writing JPEG files). I will use options to allow picking optional features (like image-format support).

I highly suggest you supply a buildtype option or it will be painfully slow. By default, it uses "--buildtype debug" which provides no optimization options at all. I highly suggest using either the debugoptimized buildtype (equivalent to -O2 -g), the release buildtype (equivalent to -O3), or maybe minsize if you are space/cache-constrained (equivalent to -Os -g).

Even the debugoptimized buildtype reduced image generation time from 51 seconds to 12.7 seconds (as of Ch. 13.2). The release buildtype brought it down to 12.5 seconds. Minsize resulted in 14.4 seconds on my system, so it may not make as much sense for you (because it won't do some things that can increase size like loop unrolling).

I have added a wrap dependency system to help support Windows builds. If you don't have pkg-config/pkgconf installed, and libpng/libjpeg-turbo aren't available systemwide, you may want to set `--wrapmode=forcefallback` to make dependency resolution faster. If you want to only use system dependencies, use the `--wrapmode=nofallback` option (which disables fetching external dependencies and only uses system dependencies).

If it is set up to build libpng and libjpeg-turbo as DLLs, you may have to use `meson devenv` to get a shell to run it, or it will fail to find the necessary DLLs. This doesn't apply if linking against system libraries (or statically). You can set it up to build them as DLLs by giving the option default\_library=shared (as in `meson setup BUILDDIR -D default_library=shared`). To make this easier, it is set up to build subprojects as static by default.

Build options can be passed to dependencies like `meson setup -D libjpeg-turbo:tests=enabled`.

The performance of writing to a terminal is much worse on Windows (tested in Windows Terminal), but this seems unavoidable.

### Speed Test: Final version ###
The file has 500 samples per pixel, at 1200×675 px, and outputs in PPM format.

 - plain (No options): 15 hours 24 minutes 6.34 seconds
 - debug (-O0 -g): 15 hours 24 minutes 6.32 seconds
 - debugoptimized (-O2 -g): 2 hours 9 minutes 35.94 seconds
 - debugoptimized (-O2 -g), 4 threads: 46 minutes 48.51 seconds (almost 3 times as fast)
 - release (-O3): 2 hours 9 minutes 6.97 seconds
 - minsize (-Os -g): 2 hours 14 minutes 6.38 seconds

## Enhancements ##
I have enhanced it so that it can take arguments specifying an output file, supporting PPM, BMP, and (optionally) PNG and JPEG.

I have some quirks to help support Windows, but I'm still testing Windows support.

## Results ##
This is rendered at 1200×675 px, 500 samples per pixel (see `raytracer.c++`), and output in JPEG (quality 95). The renders are stored in the `examples/` folder.

![A scene with many different orbs of different sizes. There are 2 large orbs and many small ones. Some of the orbs are opaque. Some of them have a glassy, reflective look. Some of them have a reflective look closer to metal. The non-glassy orbs are in various colors, such as brown, red, blue, green, and purple.](examples/final.jpg)
