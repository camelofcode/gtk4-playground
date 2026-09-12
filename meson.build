project(
  'gtk4-playground',
  'c',
  version: '1.0',
  default_options: [
    'c_std=c17',
    'warning_level=2',
  ]
)

gtk = dependency('gtk4', version: '>=4.10')

json_glib = dependency('json-glib-1.0', version: '>=1.6')

gnome = import('gnome')

resources = gnome.compile_resources(
  'gtk4-playground-resources',
  './resources/resources.gresource.xml',
  source_dir: '.'
)

sources = files(
  'src/main.c',
  'src/core/task.c',
  'src/core/task_storage.c',
  'src/ui/windows/main_window.c',
  'src/ui/windows/text_binding_window.c',
  'src/ui/windows/task_list_window.c',
  'src/ui/widgets/task_row.c',
)

executable(
  'gtk4-playground',
  sources,
  resources,
  dependencies: [gtk, json_glib],
)

storage_test = executable(
  'task-storage-test',
  'tests/task_storage_test.c',
  'src/core/task.c',
  'src/core/task_storage.c',
  include_directories: include_directories('.'),
  dependencies: json_glib,
)
test('task-storage', storage_test)

window_test = executable(
  'task-window-test',
  'tests/task_window_test.c',
  'src/core/task.c',
  'src/core/task_storage.c',
  'src/ui/widgets/task_row.c',
  'src/ui/windows/task_list_window.c',
  'src/ui/windows/main_window.c',
  'src/ui/windows/text_binding_window.c',
  resources,
  include_directories: include_directories('.'),
  dependencies: [gtk, json_glib],
)
# Requires a display; skipped automatically when one is unavailable.
test('task-window', window_test, env: ['G_DEBUG=fatal-warnings'])
