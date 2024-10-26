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

rtlib = static_library('rtlib',
                       rt_lib_files,
                       include_directories: [internal_include, sys_include],
                       dependencies: deps,
                       install: true)
