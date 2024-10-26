# This implements the library portion of raytracer internals
# It is relative to active subdirectory.
rt_lib_files = files('args.c++',
                     'bitmap.c++',
                     'camera.c++',
                     'hittable-list.c++',
                     'interval.c++',
                     'material.c++',
                     'quirks.c++',
                     'sphere.c++')

# Only used internally in library portion
internal_include = include_directories('internal')

configure_file(output: 'rt-lib-config.h', configuration: conf_data)

# Note: this doesn't seem to have a method for noting dependencies within rtlib itself.
rtlib = static_library('rtlib',
                       rt_lib_files,
                       include_directories: [internal_include, sys_include],
                       dependencies: deps,
                       install: true)

# I generate a pkgconfig file because rtlib.a itself does *not* record its
# dependencies, and it would give link failures if used externally.
pkg = import('pkgconfig')

pkg.generate(rtlib)
