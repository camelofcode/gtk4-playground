# GTK 4 playground

Small C/GTK 4 examples, built with Meson and Ninja.

## Dependencies

You need a C17 compiler, Meson, Ninja, pkg-config, GTK 4.10 or newer,
and JSON-GLib 1.6 or newer (including development headers).

On Fedora:

```sh
sudo dnf install gcc meson ninja-build pkgconf-pkg-config gtk4-devel json-glib-devel
```

If the existing project already builds, the new dependency alone is:

```sh
sudo dnf install json-glib-devel
```

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
