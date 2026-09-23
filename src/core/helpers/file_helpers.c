#include "file_helpers.h"
#include <errno.h>
#include <glib/gstdio.h>
#include <string.h>

gboolean file_helpers_write_all_bytes(
    const gchar *path,
    const void *contents,
    gsize length,
    GError **error
) {
    g_autofree gchar *directory = g_path_get_dirname(path);
    if (g_mkdir_with_parents(directory, 0700) < 0) {
        g_set_error(
            error,
            G_FILE_ERROR,
            g_file_error_from_errno(errno),
            "Could not create parent directory: %s",
            g_strerror(errno)
        );
        return FALSE;
    }
    g_autoptr(GFile) file = g_file_new_for_path(path);
    return g_file_replace_contents(
        file,
        contents,
        length,
        NULL,
        FALSE,
        G_FILE_CREATE_PRIVATE,
        NULL,
        NULL,
        error
    );
}

gboolean file_helpers_write_all_text(
    const gchar *path,
    const gchar *contents,
    GError **error
) {
    return file_helpers_write_all_bytes(path, contents, strlen(contents), error);
}

gchar *file_helpers_read_all_text(const gchar *path, GError **error) {
    g_autoptr(GFile) file = g_file_new_for_path(path);
    gchar *contents = NULL;
    if (!g_file_load_contents(file, NULL, &contents, NULL, NULL, error)) {
        return NULL;
    }
    return contents;
}
