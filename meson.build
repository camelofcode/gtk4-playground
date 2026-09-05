project(
  'gtk-playground',
  'c',
  version: '1.0',
  default_options: [
    'c_std=c17',
    'warning_level=2',
  ]
)

gtk = dependency('gtk4')

gnome = import('gnome')

resources = gnome.compile_resources(
  'gtk-playground-resources',
  'resources.gresource.xml',
  source_dir: '.'
)

sources = files(
  'main.c',
  'main_window.c',
  'text_binding_window.c',
  'task_list_window.c',
)

executable(
  'gtk-playground',
  sources,
  resources,
  dependencies: gtk,
)