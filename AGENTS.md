# GTK4 Playground Guidance

This repository is a learning project. Prefer explicit, readable C and small,
well-named components over compressed or clever implementations.

## Work from established patterns

- Before adding or changing a component, inspect the nearest equivalent in
  this repository and preserve its file organization, ownership, lifecycle,
  signal, UI-template, resource, build, and testing patterns.
- Treat departures from an established pattern as deliberate design changes.
  Explain and test them rather than silently introducing a second convention.
- Use `.agents/skills/gtk-component-change/SKILL.md` when adding or
  substantially changing a window, widget, model, or feature that crosses
  multiple layers.

## Reuse and duplication

- Before writing a new function, helper, model, widget, parser, serializer, or
  adapter, search the repository for the same responsibility. Search by likely
  names, the relevant data types, and the underlying library calls; a naming
  mismatch does not make duplicate behavior acceptable.
- Reuse an existing implementation when it owns the behavior. If it is almost
  suitable, generalize or relocate it and migrate callers instead of creating
  a parallel implementation.
- Treat semantically duplicated logic as a defect during implementation and
  review, even when the functions have different names or live in different
  feature directories. A new shared helper is not complete while equivalent
  private implementations remain.
- Keep abstractions responsibility-based. Do not merge code that merely looks
  similar when its ownership, invariants, or expected evolution are genuinely
  different; document that distinction when it would otherwise look like
  accidental duplication.
- During final review, inspect added logic against existing helpers and nearby
  components, not only for textual duplication within the current diff.

## Code Review Rules

- Flag newly introduced semantic duplication as a defect. The safe path is to
  reuse the owning implementation or generalize it and migrate equivalent
  callers; accept separate implementations only when their differing ownership
  or invariants are explicit.

## C style

- Run the repository's `.clang-format` over every changed C and header file
  before completing the task.
- Use descriptive names for parameters, variables, properties, callbacks, and
  state. Single-letter names are acceptable only for conventional, tiny loop
  indices.
- Prefer explicit functions to code-generating macros for ordinary getters,
  setters, and repeated small operations. Standard GLib/GObject declaration
  and type macros are expected.
- Keep GObject property names and metadata useful and minimal. Do not add long
  nicknames or blurbs that merely restate the identifier.
- Avoid broad mechanical substitutions that can alter string literals, format
  specifiers, identifiers with different meanings, or unrelated code.

## Component boundaries

- Put domain objects, persistence, API clients, and parsing in `src/core/`.
  Give each substantial model type its own `.c` and `.h` files under
  `src/core/models/`, including domain models such as tasks and filesystem
  nodes as well as models populated from APIs.
- Put reusable, domain-independent core utilities in `src/core/helpers/` and
  group them by a single logical responsibility such as JSON, GObject, or file
  handling. Do not create feature-specific helper files or catch-all utility
  modules that mix unrelated responsibilities.
- Put reusable view components in `src/ui/widgets/`. A custom widget normally
  owns a matching `.c`, `.h`, and `ui/widgets/*.ui` template.
- Keep `src/ui/windows/` focused on window-level coordination. Do not build
  reusable rows, paging controls, or other independent components inline in a
  window module.
- Put UI-facing model adapters in `src/ui/models/` and shared UI helpers in
  `src/ui/` only when more than one component can use them.
- When behavior is domain-independent, prefer a reusable component or model
  over an article-, task-, or screen-specific copy.

## GTK and UI integration

- This is GTK 4.10 or newer. Use GTK 4 APIs and lifecycle signals; do not copy
  GTK 3 calls or GtkBuilder properties without checking their GTK 4 form.
- Keep secondary-window lifecycle consistent with the existing windows:
  register the window with the application, set the main window as its
  transient parent, hide reusable windows on close, and destroy them with the
  main window. Test any intentional exception against application shutdown and
  reactivation.
- Custom template widgets must release their template children with
  `gtk_widget_dispose_template()` during disposal.
- Design windows for small screens: use scrolling where content can grow,
  avoid unnecessary minimum sizes, keep windows resizable, and preserve image
  aspect ratios.
- Every new or renamed `.ui` file must be handled in all applicable places:
  `resources.gresource.xml`, `meson.build`, the relevant builder/template code,
  tests, and `design/gtk4-playground.cmb`. Refresh the Cambalache entry hash
  after changing a listed UI file.

## Data and serialization

- Prefer JSON-GLib's GObject deserialization over manual field-by-field JSON
  parsing. Add a small `JsonSerializable` adapter only for mappings the library
  cannot perform directly, such as a JSON array exposed as a `GListStore`.
- Keep transport concerns in the API client and model concerns in model files.
  Do not make windows parse response JSON.
- Make asynchronous ownership explicit. A callback must not retain an unsafe
  raw pointer to window state that can be destroyed while the request is
  pending.

## Verification and completion

- Add or update focused tests for models, serialization, reusable components,
  and window lifecycle whenever the change can regress those behaviors.
- A successful compile is not sufficient for UI work. Run the relevant tests,
  load or validate changed UI resources, and exercise the affected runtime flow
  when a display backend is available.
- If the shell has no display, check for a usable headless backend before
  declaring runtime verification impossible.
- Review the final diff as an integrated feature: source, headers, templates,
  resources, Cambalache project, Meson targets, documentation, and tests.
- Local tests use disposable fixtures and have no production access. Run them,
  fix failures caused by the requested change, and rerun affected tests without
  asking for separate permission.
