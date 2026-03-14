
#
# see https://mesonbuild.com/Gnome-module.html
gnome = import('gnome')
windows = import('windows')

app_resources = gnome.compile_resources(
    'resources', 'fract.gresources.xml'
    , source_dir: meson.current_source_dir()
)

dataDir = join_paths(get_option('prefix'), get_option('datadir'))
iconDir = join_paths(dataDir, 'icons/hicolor')
install_data(
      meson.project_name() + '.png'
    , install_dir: join_paths(iconDir, '64x64/apps'))
install_data(
      meson.project_name() + '.desktop'
    , install_dir: join_paths(dataDir, 'applications'))

win_res_file = []
if target_machine.system() == 'windows'     # expects resource compiler, disable if you don't have one (part of msys2 but not claimed by any package... )
    win_res_file = windows.compile_resources('win.rc'
                            ,args: []
                            ,depend_files: [meson.project_name() + '.ico']
                           )

endif