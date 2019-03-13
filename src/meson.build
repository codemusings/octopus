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
    'oct-application.c',
    'oct-cell-renderer-button.c',
    'oct-task-view.c',
    'oct-topic-view.c'
]
deps = [
    dependency('gtk+-3.0'),
    dependency('sqlite3')
]
executable('octopus', res, install: true, sources: srcs, dependencies: deps)
