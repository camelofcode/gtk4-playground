# Regression Lessons

Use this list when reviewing a feature that spans models, API calls, widgets,
windows, or UI resources. It records concrete failure modes previously seen in
the companion GTK playground and the safeguard each one requires.

- Formatting was applied only after review feedback. Discover and use
  `.clang-format` before writing enough code to drift from it.
- Single-letter identifiers, macro-generated accessors, and redundant GObject
  metadata made learning-oriented code difficult to read. Prefer descriptive
  names, explicit functions, and minimal metadata.
- A broad identifier substitution changed a format string from `%s` to an
  invalid token. Do not perform blind global replacements; inspect the diff and
  rebuild after mechanical edits.
- Manual field-by-field JSON parsing duplicated JSON-GLib. Let JSON-GLib map
  ordinary properties and write narrow adapters only for unsupported mappings.
- A new model utility reimplemented the existing JSON list deserializer almost
  exactly. Before writing reusable behavior, search by responsibility, relevant
  types, and underlying library calls; reuse or generalize the existing owner
  and remove parallel implementations.
- Multiple models were placed in one large source/header pair. Keep substantial
  model types in separate files and a focused model directory.
- Row widgets and their layouts were built inside a window module. Give reusable
  widgets their own C, header, and UI-template files; keep windows as
  coordinators.
- Paging behavior was embedded in one window and its page model was tied to one
  endpoint. Extract reusable UI behavior and use domain-neutral core types.
- Fixed image dimensions distorted content. Preserve aspect ratio and treat
  dimensions as maximum bounds.
- The initial layout did not fit a small screen and lacked appropriate
  scrolling. Exercise responsive sizing rather than assuming a large display.
- Removing a transient-parent relationship to improve maximization broke
  shutdown: the hidden child remained an application window. Preserve ownership
  and verify close, main-window destruction, process exit, and reactivation.
- A property from the wrong GTK version passed compilation boundaries but
  failed when GtkBuilder loaded the UI. Validate resources and confirm every API
  and property against GTK 4 rather than copying GTK 3 code.
- Refactoring collection deserialization broke paged items and nested comments.
  Add regression tests before or with the refactor, including nested data.
- The first implementation added no tests and explicitly stopped after a
  successful compile. Compilation alone is not completion for UI or parsing
  work; run focused tests, the full suite, and the affected runtime flow.
- Lack of a normal display was treated as the end of runtime verification even
  though a headless backend was available. Check the installed GTK backends.
- UI resources were added without updating the Cambalache project. Treat the
  `.cmb` file, resource manifest, builder/template code, Meson target, and tests
  as one integration set.
