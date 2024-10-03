project('raytrace', 'c', 'cpp', default_options: ['cpp_std=c++20'], license: 'MIT', version: '1.1')

png_dep = dependency('libpng', required: get_option('png'))

conf_data = configuration_data()
conf_data.set('ENABLE_PNG', png_dep.found(), description: 'Enable libpng support')
configure_file(output: 'config.h', configuration: conf_data)

deps = [png_dep]

# The files implementing the raytracer internals
rt_main_files = ['args.c++',
                 'bitmap.c++',
                 'camera.c++',
                 'hittable-list.c++',
                 'interval.c++',
                 'material.c++',
                 'quirks.c',
                 'sphere.c++']

# I don't need most of this stuff for ppm-example, just link the necessary ones.
executable('ppm-example', 'ppm-example.c++', 'bitmap.c++', dependencies: png_dep)
executable('render-example', 'render-example.c++', 'args.c++', 'bitmap.c++', 'quirks.c', dependencies: deps)
executable('raytracer', 'raytracer.c++', rt_main_files, dependencies: deps)
