project('raytrace', 'c', 'cpp', default_options: ['cpp_std=c++20'], license: 'MIT', version: '1.1')

# The files implementing the raytracer internals
rt_main_files = ['bitmap.c++',
                 'camera.c++',
                 'hittable-list.c++',
                 'interval.c++',
                 'material.c++',
                 'quirks.c',
                 'sphere.c++']

# I don't need most of this stuff for ppm-example, just link the necessary ones.
executable('ppm-example', 'ppm-example.c++', 'bitmap.c++', 'interval.c++')
executable('render-example', 'render-example.c++', 'args.c++', 'bitmap.c++', 'quirks.c')
executable('raytracer', 'raytracer.c++', rt_main_files)
