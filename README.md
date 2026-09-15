# GTK 4 playground

Small C/GTK 4 examples, built with Meson and Ninja.

## Dependencies

You need a C17 compiler, Meson, Ninja, pkg-config, GTK 4.10 or newer,
JSON-GLib 1.6 or newer (including development headers), and a Sass compiler.

On Fedora:

```sh
sudo dnf install gcc meson ninja-build pkgconf-pkg-config gtk4-devel json-glib-devel sassc
```

If the existing project already builds, the Sass dependency alone is:

```sh
sudo dnf install sassc
```

Fedora packages `sassc`, but its LibSass engine is
[end-of-life](https://sass-lang.com/blog/libsass-is-end-of-life/).
For new installations, prefer [Dart Sass](https://sass-lang.com/install/).
One way to install it without a system-wide npm installation is:

```sh
sudo dnf install npm
npm install --global --prefix "$HOME/.local" sass
export PATH="$HOME/.local/bin:$PATH"
```

Keep that PATH entry in your shell configuration. Meson prefers `sass` (Dart
Sass), falling back to `sassc`. After changing compilers, run
`meson setup --reconfigure builddir`.

## UI layouts

`ui/windows/` contains one GtkBuilder file per window. The application loads
all three into the same builder before connecting behaviour in C.
`ui/widgets/task-row.ui` contains the reusable task-row widget template.
`resources.gresource.xml` at the project root lists the embedded assets.

Open the `.cmb` project in `design/` to edit all four layouts in Cambalache.
Its file paths are relative to the design project. Reload the project if it
was already open when the layouts were reorganized.

## Styles

Styles live in `ui/sass/`: `app.scss` is the entry point that lists
imports, and `_task-list-window.scss` contains the task window styles.
The leading underscore marks a Sass partial, included by the entry point.

Edit the partial to change task styles. Meson compiles `app.scss` to `builddir/app.css` and embeds
that generated CSS in the application; Sass is only needed at build time.
Normal `meson compile -C builddir` builds include stylesheet changes.
The resource path is `ui/app.css`.

SCSS supports nesting, for example `&.complete` inside `.task-row` produces
`.task-row.complete`. GTK still requires GTK-compatible CSS: Sass does not
add browser-only styling features to GTK. GTK's `alpha()` conflicts with a
Sass function, so interpolation such as `#{"alpha(currentColor, 0.3)"}`
passes it through unchanged for GTK to evaluate.

To add another window's styles, create a `_window-name.scss` partial, add
its import to `ui/sass/app.scss`, and list the partial in the CSS
custom target's `depend_files` in `meson.build`. This ensures edits to
partials trigger recompilation.

The entry point uses `@import` for compatibility with `sassc`. Dart Sass
deprecates `@import` in favor of `@use`; switching to `@use` requires
dropping the `sassc` fallback.

## Build and run

From the repository root:

```sh
meson setup builddir
meson compile -C builddir
./builddir/gtk4-playground
```

For an existing build directory, start with `meson compile -C builddir`;
Meson regenerates its configuration when `meson.build` changes.

Run the persistence tests with:

```sh
meson test -C builddir --print-errorlogs
```

The GTK window integration test opens and closes test windows using temporary
data. It is skipped when no display is available; the core storage tests run
without a display.

## Task data

`src/core/task.c` implements a GTK-independent Task GObject with `text` and
`done` properties. A GListStore holds the tasks; the task window binds that
collection to rows, which observe and update their Task objects.

`src/core/task_storage.c` reads and writes versioned JSON using JSON-GLib.
The file is `$XDG_DATA_HOME/gtk4-playground/tasks.json`, normally
`~/.local/share/gtk4-playground/tasks.json`. Task order and completion state
are preserved.

Tasks load when the application creates its task collection. Closing the task
window saves the collection and hides the window; reopening it reuses the
in-memory collection. Application shutdown also saves, including when the main
window closes while the task window is open. Saves replace the file using GIO.
There is no autosave, so a crash or forced termination can lose unsaved changes.

A missing file starts an empty list. An unreadable, malformed, or unsupported
file shows an error and disables task editing for that run, preserving the
original file. Repair or move that file and restart to resume editing. A failed
save when closing either window shows an error and cancels closing so you can
retry. The shutdown fallback logs failures if the windows have already been
destroyed. GTK 4 uses asynchronous alert dialogs and close-request handlers.
