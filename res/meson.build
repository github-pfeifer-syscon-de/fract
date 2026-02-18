
#
# see https://mesonbuild.com/Gnome-module.html
gnome = import('gnome')

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
