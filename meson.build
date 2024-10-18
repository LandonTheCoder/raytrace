project('raytrace', 'c', 'cpp', license: 'MIT', version: '1.3.1',
        meson_version: '>= 0.59.0',
        default_options: ['cpp_std=c++20', 'default_library=static'])
# I put the default options up there because it doesn't seem to work otherwise.
# I want to avoid DLLs because it makes it annoying to work with on Windows
# because "DLL not found".

conf_data = configuration_data()

png_dep = dependency('libpng', required: get_option('png'), fallback: 'libpng')

# This selects which JPEG writer implementation to use.

use_tjpeg = get_option('turbojpeg')

# TurboJPEG takes precedence because the code is simpler
use_libjpeg = get_option('jpeg').disable_auto_if(use_tjpeg.enabled())

jpeg_dep = dependency('libturbojpeg', required: use_tjpeg, version: '>= 3.0')

conf_data.set('ENABLE_TJPEG3', jpeg_dep.found(), description: 'Enable TurboJPEG 3 support')

# This is false unless we fall back to libjpeg
conf_data.set('ENABLE_LIBJPEG', false, description: 'Enable libjpeg support')

if not jpeg_dep.found()
    if use_tjpeg.enabled()
        # Do fallback here, I don't do it by default so that we get system dependencies
        # on distros lacking TurboJPEG 3 but having libjpeg-turbo.
        jpeg_dep = dependency('libturbojpeg', required: use_tjpeg, version: '>= 3.0',
                              fallback: 'libjpeg-turbo')
        conf_data.set('ENABLE_TJPEG3', jpeg_dep.found(), description: 'Enable TurboJPEG 3 support')
    else
        # Correct version of turbojpeg not found, fall back to libjpeg.
        jpeg_dep = dependency('libjpeg', required: use_libjpeg, fallback: 'libjpeg-turbo')
        conf_data.set('ENABLE_LIBJPEG', jpeg_dep.found(), description: 'Enable libjpeg support')
    endif
endif

conf_data.set('ENABLE_PNG', png_dep.found(), description: 'Enable libpng support')
configure_file(output: 'config.h', configuration: conf_data)

deps = [png_dep, jpeg_dep]

# Files linked in which are OS-specific
os_inputs = []

# Compiler-specific arguments. (MSVC won't take .c++ files without being told to.)
cpp_args = []

# OS-specific stuff
if host_machine.system() == 'windows'
    windows = import('windows')
    resource_depends = ['arch/win/raytracer.exe.manifest']
    resource_file = windows.compile_resources('arch/win/raytracer.rc',
                                              depend_files: resource_depends)
    os_inputs += resource_file
    cxx = meson.get_compiler('cpp')
    if cxx.get_argument_syntax() == 'msvc'
        # All files on command line are C++ (including *.c++)
        cpp_args += '/TP'
    endif
endif

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
executable('render-example',
           'render-example.c++',
           'args.c++',
           'bitmap.c++',
           'quirks.c',
           os_inputs,
           cpp_args: cpp_args,
           dependencies: deps)
executable('raytracer', 'raytracer.c++', rt_main_files, os_inputs, cpp_args: cpp_args, dependencies: deps)
