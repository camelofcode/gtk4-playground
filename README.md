# GTK 4 playground

[![CI](https://github.com/camelofcode/gtk4-playground/actions/workflows/ci.yml/badge.svg)](https://github.com/camelofcode/gtk4-playground/actions/workflows/ci.yml)

Small C/GTK 4 examples, built with Meson and Ninja.

## Why this exists

I built this repository as a learning exercise after moving my day-to-day
development to Linux and wanting to understand GTK at its native C layer. The
example applications are deliberately small: their purpose is to exercise
architecture, lifecycle, data, networking, persistence, resources, and testing
rather than to become useful end-user products.

I am making the code public as an educational reference and as inspectable
evidence of that learning. It aims to demonstrate clean, explicit patterns for
structuring GTK and C code, including separation between declarative layout,
UI behaviour, reusable widgets, and GTK-independent core logic. These are
patterns I have found clear and maintainable, not a claim that they are the one
official way to build a GTK application. Corrections and better alternatives
are welcome.

## Project structure

```text
.
├── meson.build                 # Build targets and dependencies
├── resources.gresource.xml     # Manifest of embedded application assets
├── design/                     # Cambalache project (.cmb)
├── src/
│   ├── main.c                  # Application startup and UI loading
│   ├── core/                   # Models, API client, and JSON persistence
│   │   ├── models/             # One GObject model per .c/.h pair
│   │   └── helpers/            # Reusable helpers grouped by responsibility
│   └── ui/
│       ├── windows/            # Window behaviour and signal connections
│       ├── widgets/            # Reusable widget implementations
│       ├── gtk_helpers.c/.h    # Shared GTK helpers
│       └── error_dialog.c/.h   # Error reporting UI
├── ui/
│   ├── windows/                # One .ui layout per window
│   ├── widgets/                # Reusable .ui widget templates
│   └── sass/                   # SCSS entry point and style partials
├── tests/                      # Model, storage, and window integration tests
└── builddir/                   # Generated build output (not committed)
```

Layout belongs in `.ui` files, behaviour in C, and styling in SCSS.
The `ui/` and `src/ui/` folders mirror each other: for example,
`ui/windows/task-list-window.ui` defines the layout and
`src/ui/windows/task_list_window.c` connects its behaviour. GTK-independent
model and persistence code lives in `src/core/`.

## Dependencies

You need a C17 compiler, Meson, Ninja, pkg-config, GTK 4.10 or newer,
JSON-GLib 1.6 or newer, libsoup 3, and a Sass compiler. Development headers
are needed for each library.

On Fedora:

```sh
sudo dnf install gcc meson ninja-build pkgconf-pkg-config gtk4-devel json-glib-devel libsoup3-devel sassc
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
all window layouts into the same builder before connecting behaviour in C.
`ui/widgets/task-row.ui` contains the reusable task-row widget template.
`resources.gresource.xml` at the project root lists the embedded assets.

Open the `.cmb` project in `design/` to edit the layouts in Cambalache.
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

`src/core/models/task.c` implements a GTK-independent Task GObject with `text` and
`done` properties. A GListStore holds the tasks; the task window binds that
collection to rows, which observe and update their Task objects.

`src/core/task_storage.c` reads and writes a plain JSON array of tasks using JSON-GLib.
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

## Articles API example

The **Articles API** window demonstrates a paginated REST client for the
example Articles API. It fetches each page asynchronously, shows summaries,
and opens article details and comments in a second view in the same window.
The previous/next controls use the server's `totalPages`, so paging is not
limited to a fixed number of pages. The current list page remains in place
while viewing an article.

The API client uses libsoup 3 and JSON-GLib. A self-contained WireMock backend
is included in `wiremock/`; its responses cover two list pages, article
details, nested comments, and local image loading. Running it requires Podman
and `podman-compose`.

Start it from the repository root with Podman Compose:

```sh
podman-compose -f wiremock/compose.yaml up --detach
```

The application uses `http://localhost:8090` by default, so no additional
configuration is needed. The WireMock administration interface is available at
`http://localhost:8090/__admin/` while the container is running. Stop and
remove the container with:

```sh
podman-compose -f wiremock/compose.yaml down
```

To use another compatible server, set `ARTICLES_API_BASE_URL`, for example:

```sh
ARTICLES_API_BASE_URL=http://localhost:8090 ./builddir/gtk4-playground
```

The client expects `GET /api/articles?page=N` and
`GET /api/articles/{id}`. Article summary, detail, page, and comment values
are GObjects in `src/core/models/`; each model has its own `.c` and `.h`
file. A detail owns a `GListModel` of comment objects. JSON-GLib deserializes
the model properties, while the shared JSON helpers convert JSON arrays into
`GListStore` collections. Helpers in `src/core/helpers/` are grouped by the
responsibility they serve rather than by the feature that first uses them.
Image URLs are fetched asynchronously and decoded as `GdkTexture` objects.
GTK 4 `GtkPicture` widgets display those textures with contained aspect-ratio
scaling.

The fixture data is deliberately generic and the bundled placeholder artwork
is original to this repository, so the complete example can be shared and run
without depending on private services or third-party media.

## Licence

This learning project is available under the [MIT Licence](LICENSE).
