gnome = import('gnome')
res = gnome.compile_resources(
    'oct-resources',
    '../res/de.codemusings.Octopus.gresource.xml',
    source_dir: '../res',
    c_name: 'oct'
)

srcs = [
    'main.c',
    'octopus.c',
    'oct-cell-renderer-button.c',
#    'oct-resources.c',
    'oct-settings-popover.c',
    'oct-sync.c',
    'oct-task-view.c',
    'oct-topic-view.c'
]
deps = [
    dependency('gtk+-3.0'),
    dependency('json-glib-1.0'),
    dependency('libcurl'),
    dependency('sqlite3')
]
executable('octopus', res, install: true, sources: srcs, dependencies: deps)
