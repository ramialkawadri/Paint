paint_sources = [
  'canvas-region-snapshot.c',
  'canvas-region.c',
  'main.c',
  'paint-application.c',
  'paint-window.c',
  'toolbar.c',
]

subdir('drawing-tools')
subdir('utils')

paint_deps = [
  dependency('gtk4'),
  dependency('libadwaita-1', version: '>= 1.4'),
  cc.find_library('m', required : false),
]

paint_sources += gnome.compile_resources('paint-resources',
  'paint.gresource.xml',
  c_name: 'paint'
)

executable('paint', paint_sources,
  dependencies: paint_deps,
       install: true,
)
