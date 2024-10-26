project('raytrace', 'c', 'cpp', license: 'MIT', version: '2.0',
        meson_version: '>= 0.59.0',
        default_options: ['cpp_std=c++20', 'default_library=static'])
# I put the default options up there because it doesn't seem to work otherwise.
# I want to avoid DLLs because it makes it annoying to work with on Windows
# because "DLL not found".

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

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
# I generate config file for internal use in lib subdir

deps = [png_dep, jpeg_dep]

# Files linked in which are OS-specific
# Note: I can make them subdir-independent by using files()
os_inputs = []

# OS-specific stuff
if host_machine.system() == 'windows'
    windows = import('windows')
    resource_depends = ['arch/win/raytracer.exe.manifest']
    resource_file = windows.compile_resources('arch/win/raytracer.rc',
                                              depend_files: resource_depends)
    os_inputs += resource_file
    if cxx.get_argument_syntax() == 'msvc'
        # All files on command line are C++ (including *.c++)
        # MSVC thinks that file.c++ is an object file, set it straight here.
        add_project_arguments('/TP', language: ['cpp'])
    endif
endif

# Exposed externally
sys_include = include_directories('include')

subdir('lib')

# Note: This will create files as builddir/src/*
#subdir('src')

# Build demonstration executables
executable('gradient-test',
           'src/gradient-test.c++',
           os_inputs,
           include_directories: [sys_include],
           link_with: rtlib,
           install: true)
executable('raytracer',
           'src/raytracer.c++',
           os_inputs,
           include_directories: [sys_include],
           link_with: rtlib,
           install: true)
