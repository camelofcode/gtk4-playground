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

executable(
  'gtk-playground',
  [
    'main.c',
    'main_window.c',
    'text_binding_window.c',
  ],
  dependencies: gtk,
)