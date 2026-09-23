#ifndef FILE_HELPERS_H
#define FILE_HELPERS_H

#include <gio/gio.h>

/* Creates or overwrites a file with exactly length bytes, without a backup.
 * Creates missing parent directories with mode 0700.
 * New files are private to the current user.
 * Returns FALSE and sets error on failure; contents remains caller-owned.
 */
gboolean file_helpers_write_all_bytes(
    const gchar *path,
    const void *contents,
    gsize length,
    GError **error
);

/* Writes NUL-terminated text using the same behavior as write_all_bytes.
 * Calculates the byte length internally; the terminating NUL is not written.
 */
gboolean file_helpers_write_all_text(
    const gchar *path,
    const gchar *contents,
    GError **error
);

/* Returns owned, NUL-terminated file contents, or NULL with error on failure. */
gchar *file_helpers_read_all_text(const gchar *path, GError **error);

#endif
