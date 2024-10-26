# These headers are externally exposed.
sys_include = include_directories('.')

public_headers = files('rt/color.h',
                       'rt/args.h',
                       'rt/bitmap.h',
                       'rt/vec3.h',
                       'rt/rtweekend.h',
                       'rt/interval.h',
                       'rt/ray.h',
                       'rt/material.h',
                       'rt/hittable.h',
                       'rt/hittable-list.h',
                       'rt/quirks.h',
                       'rt/camera.h',
                       'rt/utils.h',
                       'rt/sphere.h')

install_headers(public_headers,
                preserve_path: true)
