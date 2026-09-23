#ifndef TASK_STORAGE_H
#define TASK_STORAGE_H

#include <gio/gio.h>

/* Load and save use the default path internally.
 * The returned model is owned by the caller. Missing files load empty.
 * Invalid files return NULL; callers must not overwrite them with empty data.
 */
GListStore *task_storage_load(
    GError **error
);
gboolean task_storage_save(
    GListModel *tasks,
    GError **error
);

#endif
