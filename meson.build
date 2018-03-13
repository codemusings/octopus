project('octopus', 'c')
subdir('src')

install_data(
    'res/de.codemusings.Octopus.gschema.xml',
    install_dir: join_paths(get_option('datadir'), 'glib-2.0', 'schemas')
)
install_data(
    'res/de.codemusings.Octopus.desktop',
    install_dir: join_paths(get_option('datadir'), 'applications')
)
meson.add_install_script('meson_post_install_gsettings.py')
