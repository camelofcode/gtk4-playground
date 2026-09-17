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

sass = find_program('sass', required: false)
if sass.found()
  sass_args = ['--no-source-map', '--style=expanded']
else
  sass = find_program('sassc')
  sass_args = ['--style', 'expanded']
endif

app_css = custom_target(
  'app-css',
  input: 'ui/sass/app.scss',
  depend_files: files('ui/sass/_task-list-window.scss'),
  output: 'app.css',
  command: [sass] + sass_args + ['@INPUT@', '@OUTPUT@'],
)

resources = gnome.compile_resources(
  'gtk4-playground-resources',
  'resources.gresource.xml',
  source_dir: [meson.current_build_dir(), meson.current_source_dir()],
  dependencies: app_css,
)

sources = files(
  'src/main.c',
  'src/core/task.c',
  'src/core/task_storage.c',
  'src/core/file_system_node.c',
  'src/core/file_system_loader.c',
  'src/ui/models/file_tree_model.c',
  'src/ui/widgets/file_tree_view.c',
  'src/ui/windows/main_window.c',
  'src/ui/windows/text_binding_window.c',
  'src/ui/windows/task_list_window.c',
  'src/ui/windows/file_tree_window.c',
  'src/ui/error_dialog.c',
  'src/ui/gtk_helpers.c',
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

file_system_loader_test = executable(
  'file-system-loader-test',
  'tests/file_system_loader_test.c',
  'src/core/file_system_node.c',
  'src/core/file_system_loader.c',
  include_directories: include_directories('.'),
  dependencies: gtk,
)
test('file-system-loader', file_system_loader_test)

file_tree_view_test = executable(
  'file-tree-view-test',
  'tests/file_tree_view_test.c',
  'src/core/file_system_node.c',
  'src/core/file_system_loader.c',
  'src/ui/models/file_tree_model.c',
  'src/ui/widgets/file_tree_view.c',
  resources,
  include_directories: include_directories('.'),
  dependencies: gtk,
)
# Requires a display; skipped automatically when one is unavailable.
test('file-tree-view', file_tree_view_test, env: ['G_DEBUG=fatal-warnings'])

window_test = executable(
  'task-window-test',
  'tests/task_window_test.c',
  'src/core/task.c',
  'src/core/task_storage.c',
  'src/core/file_system_node.c',
  'src/core/file_system_loader.c',
  'src/ui/models/file_tree_model.c',
  'src/ui/widgets/file_tree_view.c',
  'src/ui/widgets/task_row.c',
  'src/ui/windows/task_list_window.c',
  'src/ui/error_dialog.c',
  'src/ui/gtk_helpers.c',
  'src/ui/windows/main_window.c',
  'src/ui/windows/text_binding_window.c',
  'src/ui/windows/file_tree_window.c',
  resources,
  include_directories: include_directories('.'),
  dependencies: [gtk, json_glib],
)
# Requires a display; skipped automatically when one is unavailable.
test('task-window', window_test, env: ['G_DEBUG=fatal-warnings'])
