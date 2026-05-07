project('fract', 'c', 'cpp'
    , default_options: ['c_std=c17'
                       , 'cpp_std=c++20'
                       , 'buildtype=release'
                       , 'warning_level=2']
    , license : 'GPL-3.0'
    , meson_version: '>=1.1.0'
    , version: '0.1')

pkgdatadir = join_paths(get_option('prefix'), get_option('datadir'), meson.project_name())
add_project_arguments('-DPACKAGE_DATA_DIR="' + pkgdatadir + '"', language: ['cpp','c'])
add_project_arguments('-DPACKAGE="' + meson.project_name() + '"', language: ['cpp','c'])
conf = configuration_data()
conf.set_quoted('FRACT_VERSION', meson.project_version())     # Surround the version in quotes to make it a C string
configure_file(output : meson.project_name() + '_config.h'
    , configuration : conf)

compile_flags = [
     '-Wno-unused-parameter'    # limit the compiler noisiness
    ,'-Wno-overlength-strings'  # this matters only if you are pedantic
]
if get_option('tune') != ''
compile_flags += [
     '-mtune='+ get_option('tune')
    ,'-march='+ get_option('tune')]
endif
cc = meson.get_compiler('c')
foreach cflag : compile_flags
    if cc.has_argument(cflag)
        add_project_arguments(cflag, language: ['c', 'cpp'])
    endif
endforeach

gtkmm3_deps     = dependency('gtkmm-3.0')
glibmm2_deps    = dependency('glibmm-2.4 giomm-2.4')
libsoup3_deps    = dependency('libsoup-3.0')
thread_deps     = dependency('threads')

deps = [gtkmm3_deps
        , glibmm2_deps
        , libsoup3_deps
        , thread_deps
]

subdir('src')
subdir('test')
subdir('res')
sources += app_resources
sources += win_res_file 

project_targt = executable(meson.project_name()
    , sources, app_resources
    , dependencies: deps
    , install: true
    , link_with: [life_lib]
    , win_subsystem: 'windows'  # disable this to see console output for windows
    )
