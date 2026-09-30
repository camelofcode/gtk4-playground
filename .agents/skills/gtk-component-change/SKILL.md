---
name: gtk-component-change
description: Implement or refactor GTK windows, widgets, models, and API-backed UI flows in this playground while preserving its component structure and verification requirements.
---

# GTK Component Change

Use the repository's existing components as the design vocabulary. Before
editing, inspect the closest window, widget, model, test, UI template, resource
entry, and Cambalache entry relevant to the requested behavior.

Read [references/repository-patterns.md](references/repository-patterns.md)
when the task adds a component, changes component boundaries, adds an API model,
or changes window lifecycle.

Read [references/regression-lessons.md](references/regression-lessons.md) when
auditing a broad feature or checking for omissions before completion.

## Shape the implementation

- Before introducing a function, helper, model, widget, parser, serializer, or
  adapter, search the repository for the same responsibility by likely names,
  relevant types, and underlying library calls. Reuse or generalize the owner
  instead of adding semantically parallel code.
- Keep domain and transport behavior in `src/core/`, reusable visual behavior
  in `src/ui/widgets/`, and orchestration in `src/ui/windows/`.
- Give independent models and widgets independent files. Keep all substantial
  domain and API GObject models in `src/core/models/`. A custom widget should
  normally have C, header, and UI-template files.
- Put reusable core helpers in `src/core/helpers/`, grouped into narrowly
  focused modules by logical responsibility such as JSON, GObject, or file
  handling. Do not use feature-specific or catch-all utility modules.
- Extract behavior that another screen could use, such as paging, instead of
  embedding a domain-specific copy in a window.
- Prefer JSON-GLib deserialization and add only the adapter required for an
  unsupported collection or property mapping.
- Preserve established window ownership and shutdown behavior while solving
  presentation concerns such as resizing or maximization.

## Integrate the whole component

Update every applicable integration surface: Meson sources and dependencies,
GResources, GtkBuilder or template initialization, the Cambalache project,
tests, and user-facing dependency documentation.

Apply `.clang-format` to changed C and headers. Use descriptive identifiers and
explicit functions; do not compress ordinary accessors behind new macros.

## Verify completion

Compile, run focused tests, then run the complete test suite. For UI changes,
also verify that resources load and exercise the affected flow with a display
backend. Use an available headless backend when the normal shell has no display.
Review the final diff for missing files and unintended architectural drift.
Also compare new logic with existing helpers and nearby components; final
review must catch semantic duplication that a text-only diff does not reveal.
