project('fract', 'c', 'cpp'
    , default_options: ['c_std=c11'
                       , 'cpp_std=c++20'
                       , 'buildtype=release'
                       , 'warning_level=2']
    , license : 'GPL-3.0'
    , meson_version: '>=1.1.0'
    , version: '0.1')

pkgdatadir = join_paths(get_option('prefix'), get_option('datadir'), meson.project_name())
add_project_arguments('-DPACKAGE_DATA_DIR="' + pkgdatadir + '"', language: 'cpp')
conf = configuration_data()
conf.set_quoted('VERSION', meson.project_version())     # Surround the version in quotes to make it a C string
configure_file(output : meson.project_name() + '_config.h'
    , configuration : conf)

subdir('src')
subdir('res')
sources += app_resources
gtkmm3_deps     = dependency('gtkmm-3.0')
glibmm2_deps    = dependency('glibmm-2.4 giomm-2.4')
thread_deps     = dependency('threads')

deps = [gtkmm3_deps
    , glibmm2_deps
    , thread_deps
    ]

project_targt = executable(meson.project_name()
    , sources, app_resources
    , dependencies: deps
    , install: true
    , win_subsystem: 'windows'  # disable this to see console output for windows
    )
