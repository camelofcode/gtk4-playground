#ifndef FILE_SYSTEM_LOADER_H
#define FILE_SYSTEM_LOADER_H

#include "models/file_system_node.h"

G_BEGIN_DECLS

FileSystemNode *file_system_loader_load(
    const gchar *path,
    GCancellable *cancellable,
    GError **error
);
void file_system_loader_load_async(
    const gchar *path,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data
);
FileSystemNode *file_system_loader_load_finish(
    GAsyncResult *result,
    GError **error
);

G_END_DECLS

#endif
