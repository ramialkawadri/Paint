paint_sources = [
  'main.c',
  'paint-application.c',
  'paint-window.c',
  'toolbar.c',
]

paint_deps = [
  dependency('gtk4'),
  dependency('libadwaita-1', version: '>= 1.4'),
]

paint_sources += gnome.compile_resources('paint-resources',
  'paint.gresource.xml',
  c_name: 'paint'
)

executable('paint', paint_sources,
  dependencies: paint_deps,
       install: true,
)
