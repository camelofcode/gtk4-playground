# GTK4 Repository Patterns

## Component map

- `src/core/`: storage, filesystem services, API clients, parsing, and other
  domain behavior with no window responsibilities.
- `src/core/models/`: all substantial domain and API GObject models, with one
  model per `.c`/`.h` pair.
- `src/core/helpers/`: reusable core utilities, split into narrowly focused
  modules by logical responsibility such as JSON, GObject, or file handling.
  Do not use this directory for feature-specific helpers or catch-all modules
  that mix unrelated responsibilities.
- `src/ui/models/`: adapters that expose core data through GTK-facing models.
- `src/ui/widgets/`: reusable custom widgets. Follow `TaskRow` and
  `FileTreeView`: public header, implementation, UI template, resource entry,
  Meson source, and focused tests where behavior warrants them.
- `src/ui/windows/`: wiring for top-level flows. Windows coordinate widgets and
  services; they do not contain the implementations of reusable child widgets.
- `ui/windows/` and `ui/widgets/`: GtkBuilder files split along the same
  boundaries as the C modules.
- `design/gtk4-playground.cmb`: every maintained UI file and the SHA-256 of its
  current contents.
- `tests/`: focused GLib tests and display-backed GTK lifecycle/component tests.

## Reuse-first checklist

Before implementing reusable behavior:

1. Search declarations and definitions for likely responsibility names.
2. Search for the important input/output types and the GLib, JSON-GLib, GTK,
   or libsoup calls the implementation would wrap.
3. Inspect `src/core/helpers/`, `src/ui/`, and the closest feature modules even
   when their filenames use different terminology.
4. If an existing function is too narrow, generalize or relocate it, update its
   tests, and migrate all equivalent callers.
5. Review the finished diff for equivalent control flow or transformations,
   not just copied text or matching function names.

One responsibility should have one owning implementation. Keep separate code
only when ownership, invariants, error semantics, or expected evolution differ;
make that distinction clear enough that it cannot be mistaken for an overlooked
duplicate.

## GTK4 lifecycle

Secondary windows are application windows owned by the main-window flow. Match
the neighboring modules:

1. Register with `gtk_window_set_application()`.
2. Set the main window with `gtk_window_set_transient_for()`.
3. Use `gtk_window_set_destroy_with_parent(window, TRUE)`.
4. Use `close-request` for close-time behavior. Allow GTK to hide a reusable
   secondary window after any required save succeeds.
5. Test that closing the secondary window hides it and closing the main window
   destroys it, allowing the application to exit.

Do not copy GTK3 `delete-event`, `gtk_widget_show_all()`, container packing, or
other removed APIs into GTK4 code. Template widgets must call
`gtk_widget_dispose_template()` from disposal.

## UI checklist

For every new or renamed UI file:

- place it in the matching `ui/windows/` or `ui/widgets/` directory;
- register it in `resources.gresource.xml`;
- load it from `src/main.c` or bind it as a widget template;
- add its owning C source to `meson.build`;
- add or refresh its entry in `design/gtk4-playground.cmb`;
- cover resource loading in a display-backed test;
- confirm every class and property is supported by GTK 4.10 or newer.

For layouts, assume a small display. Avoid fixed content sizes when natural
sizing or scrolling is appropriate. Prefer GTK4 facilities such as `GtkPicture`
when displaying scalable content while preserving aspect ratio.

## Model and API checklist

- Use one substantial GObject model per C/header pair.
- Keep ordinary fields as clear GObject properties with minimal metadata.
- Use `json_gobject_from_data()` for ordinary model deserialization.
- Implement the smallest `JsonSerializable` hook needed for collection mapping;
  deserialize collection elements through JSON-GLib as well.
- Make reusable concepts generic. Page metadata plus a typed `GListModel` is an
  `ApiPage`, not an endpoint-specific page class.
- Test valid data, nested objects or arrays, and malformed input.

## Completion evidence

At minimum, report:

- the formatter command used for changed C/header files;
- the compile result;
- focused and full test results, including display-backed results or an explicit
  reason they could not run;
- the runtime/UI validation performed;
- any integration surface intentionally left unchanged and why.
