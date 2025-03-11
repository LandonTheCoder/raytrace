# Raytrace in a Weekend #
See URL [Ray Tracing in One Weekend Series](https://raytracing.github.io), and book [Ray Tracing in a Weelend](https://raytracing.github.io/books/RayTracingInOneWeekend.html).

The basic gist of what I'm doing is:
1. Calculate the ray from the “eye” through the pixel,
2. Determine which objects the ray intersects, and
3. Compute a color for the closest intersection point.

This is based on the Ray Tracing in One Weekend tutorial, with a few modifications added for flexibility (such as modifying it so that it can output to formats other than PPM, with room for expansion).

## Code organization ##
The code is organized into a few directories: the include/ directory is public headers (following the same pattern as would be for the public directory), the lib/ directory contains code implementing the raytracing library, and the src/ directory contains example programs for demonstration. The lib/internal/ subdirectory contains headers used internally by the library portion (which aren't exposed as public API). The subprojects/ directory contains meson wrap files to allow building optional features more easily on Windows.

Example programs are built in the main meson.build, and the headers and library are handled as subdirectories.

## Build Options ##
I have 4 options:
 - The "png" feature, which enables/disables libpng support (to enable writing PNG files)
 - The "jpeg" feature, which enables/disables libjpeg/libjpeg-turbo support (which enables writing JPEG files)
 - The "turbojpeg" feature, which selects which JPEG implementation to use (preferring TurboJPEG 3 but falling back to libjpeg by default if it isn't available)
 - The "webp" feature, which enabled/disables lossless WebP support (via libwebp).

I will use options to allow picking optional features (like image-format support).

I highly suggest you supply a buildtype option or it will be painfully slow. By default, it uses "--buildtype debug" which provides no optimization options at all. I highly suggest using either the debugoptimized buildtype (equivalent to -O2 -g), the release buildtype (equivalent to -O3), or maybe minsize if you are space/cache-constrained (equivalent to -Os -g).

Even the debugoptimized buildtype reduced image generation time from 51 seconds to 12.7 seconds (as of Ch. 13.2). The release buildtype brought it down to 12.5 seconds. Minsize resulted in 14.4 seconds on my system, so it may not make as much sense for you (because it won't do some things that can increase size like loop unrolling).

I have added a wrap dependency system to help support Windows builds. If you don't have pkg-config/pkgconf installed, and the optional dependencies aren't available systemwide, you may want to set `--wrapmode=forcefallback` to make dependency resolution faster. If you want to only use system dependencies, use the `--wrapmode=nofallback` option, which disables fetching external dependencies and only uses system dependencies.

If building on Windows with libjpeg-turbo build as a wrap dependency, I suggest to set the NASM environment variable to the location of nasm before configuring. This allows libjpeg-turbo to build assembly versions of its functions for better performance. It may also work if nasm.exe is in the PATH, but the install doesn't seem to set that by default.

If it is set up to build libpng and libjpeg-turbo as DLLs, you may have to use `meson devenv` to get a shell to run it, or it will fail to find the necessary DLLs. Note that you have to do this again after every reconfigure. This doesn't apply if linking against system libraries (or statically). You can set it up to build them as DLLs by giving the option default\_library=shared (as in `meson setup BUILDDIR -D default_library=shared`). To make this easier, it is set up to build subprojects as static by default.

Build options can be passed to dependencies like `meson setup -D libjpeg-turbo:tests=enabled`.

The performance of writing to a terminal/console is much worse on Windows (tested in Windows Terminal), but this seems unavoidable. This can cause it to run slower on Windows because it has to wait on console output to finish.

### Speed Test: Final version ###
The file has 500 samples per pixel, at 1200×675 px, and outputs in PPM format. This was tested on my Linux system, results on Windows may differ. Unless specified, results are from the single-threaded version, as the initial test was before I implemented multithreading.

 - plain (No options): 15 hours 24 minutes 6.34 seconds
 - debug (-O0 -g): 15 hours 24 minutes 6.32 seconds
 - debugoptimized (-O2 -g): 2 hours 9 minutes 35.94 seconds
 - debugoptimized (-O2 -g), 4 threads: 46 minutes 48.51 seconds (almost 3 times as fast)
 - release (-O3): 2 hours 9 minutes 6.97 seconds
 - minsize (-Os -g): 2 hours 14 minutes 6.38 seconds

**Update**: I did some performance testing on a new system with more cores, and found that performance doesn't really scale beyond 4 cores/threads. As a matter of fact, performance using all threads is significantly worse than just using 4 threads, with the proportion of time spent in the kernel being *much* higher. (That new system is a desktop running Linux on an i5-12600K, testing the debugoptimized build.)

## Enhancements ##
I have enhanced it so that it can take arguments specifying an output file, supporting PPM, BMP, and (optionally) PNG and JPEG.

I have some quirks to help support Windows, but I may end up breaking Windows/MSVC build from time to time, as Windows isn't my main OS and testing it requires a reboot.

## Results ##
This is rendered at 1200×675 px, 500 samples per pixel (see `src/raytracer.c++`), and output in JPEG (quality 95). The renders are stored in the `examples/` folder.

![A scene with many different orbs of different sizes. There are 2 large orbs and many small ones. Some of the orbs are opaque. Some of them have a glassy, reflective look. Some of them have a reflective look closer to metal. The non-glassy orbs are in various colors, such as brown, red, blue, green, and purple.](examples/final.jpg)
