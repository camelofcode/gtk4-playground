#ifndef FILE_SYSTEM_NODE_H
#define FILE_SYSTEM_NODE_H

#include <gio/gio.h>

G_BEGIN_DECLS

#define FILE_SYSTEM_TYPE_NODE file_system_node_get_type()

G_DECLARE_FINAL_TYPE(
    FileSystemNode,
    file_system_node,
    FILE_SYSTEM,
    NODE,
    GObject
)

FileSystemNode *file_system_node_new(
    const gchar *name,
    const gchar *path,
    gboolean directory,
    guint64 size,
    gint64 modified_time
);

const gchar *file_system_node_get_name(FileSystemNode *self);
const gchar *file_system_node_get_path(FileSystemNode *self);
gboolean file_system_node_is_directory(FileSystemNode *self);
guint64 file_system_node_get_size(FileSystemNode *self);
guint64 file_system_node_get_file_count(FileSystemNode *self);
gint64 file_system_node_get_modified_time(FileSystemNode *self);
const GPtrArray *file_system_node_get_children(FileSystemNode *self);

void file_system_node_add_child(
    FileSystemNode *self,
    FileSystemNode *child
);
void file_system_node_sort_children(FileSystemNode *self);

G_END_DECLS

#endif
