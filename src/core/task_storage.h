#ifndef TASK_STORAGE_H
#define TASK_STORAGE_H

#include "task.h"
#include <gio/gio.h>

/* Returned path and model are owned by the caller. Missing files load empty.
 * Invalid files return NULL; callers must not overwrite them with empty data.
 */
gchar *task_storage_default_path(void);
GListStore *task_storage_load(
    const gchar *path,
    GError **error
);
gboolean task_storage_save(
    GListModel *tasks,
    const gchar *path,
    GError **error
);

#endif
