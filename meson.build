project('raytrace', 'c', 'cpp', default_options: ['cpp_std=c++20'], license: 'MIT', version: '1.2')

conf_data = configuration_data()

png_dep = dependency('libpng', required: get_option('png'))
# Future expansion: support libturbojpeg >= 3.0 and fallback to ugly code.
# For now, we just use libjpeg-turbo unconditionally.

jpeg_dep = dependency('libjpeg', required: get_option('jpeg'))

conf_data.set('ENABLE_PNG', png_dep.found(), description: 'Enable libpng support')
conf_data.set('ENABLE_LIBJPEG', jpeg_dep.found(), description: 'Enable libjpeg support')
configure_file(output: 'config.h', configuration: conf_data)

deps = [png_dep, jpeg_dep]

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
executable('ppm-example', 'ppm-example.c++', 'bitmap.c++', dependencies: deps)
executable('render-example',
           'render-example.c++',
           'args.c++',
           'bitmap.c++',
           'quirks.c',
           dependencies: deps)
executable('raytracer', 'raytracer.c++', rt_main_files, dependencies: deps)
