#include "file_system_node.h"

struct _FileSystemNode {
    GObject parent_instance;

    gchar *name;
    gchar *path;
    gboolean directory;
    guint64 size;
    guint64 file_count;
    gint64 modified_time;
    GPtrArray *children;
};

G_DEFINE_TYPE(
    FileSystemNode,
    file_system_node,
    G_TYPE_OBJECT
)

static void file_system_node_finalize(GObject *object) {
    FileSystemNode *self = FILE_SYSTEM_NODE(object);
    g_clear_pointer(&self->name, g_free);
    g_clear_pointer(&self->path, g_free);
    g_clear_pointer(&self->children, g_ptr_array_unref);
    G_OBJECT_CLASS(file_system_node_parent_class)->finalize(object);
}

static void file_system_node_class_init(FileSystemNodeClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->finalize = file_system_node_finalize;
}

static void file_system_node_init(FileSystemNode *self) {
    self->children = g_ptr_array_new_with_free_func(g_object_unref);
}

FileSystemNode *file_system_node_new(
    const gchar *name,
    const gchar *path,
    gboolean directory,
    guint64 size,
    gint64 modified_time
) {
    FileSystemNode *self = g_object_new(FILE_SYSTEM_TYPE_NODE, NULL);
    self->name = g_strdup(name);
    self->path = g_strdup(path);
    self->directory = directory;
    self->size = size;
    self->modified_time = modified_time;
    return self;
}

const gchar *file_system_node_get_name(FileSystemNode *self) {
    g_return_val_if_fail(FILE_SYSTEM_IS_NODE(self), NULL);
    return self->name;
}

const gchar *file_system_node_get_path(FileSystemNode *self) {
    g_return_val_if_fail(FILE_SYSTEM_IS_NODE(self), NULL);
    return self->path;
}

gboolean file_system_node_is_directory(FileSystemNode *self) {
    g_return_val_if_fail(FILE_SYSTEM_IS_NODE(self), FALSE);
    return self->directory;
}

guint64 file_system_node_get_size(FileSystemNode *self) {
    g_return_val_if_fail(FILE_SYSTEM_IS_NODE(self), 0);
    return self->size;
}

guint64 file_system_node_get_file_count(FileSystemNode *self) {
    g_return_val_if_fail(FILE_SYSTEM_IS_NODE(self), 0);
    return self->file_count;
}

gint64 file_system_node_get_modified_time(FileSystemNode *self) {
    g_return_val_if_fail(FILE_SYSTEM_IS_NODE(self), 0);
    return self->modified_time;
}

const GPtrArray *file_system_node_get_children(FileSystemNode *self) {
    g_return_val_if_fail(FILE_SYSTEM_IS_NODE(self), NULL);
    return self->children;
}

void file_system_node_add_child(
    FileSystemNode *self,
    FileSystemNode *child
) {
    g_return_if_fail(FILE_SYSTEM_IS_NODE(self));
    g_return_if_fail(FILE_SYSTEM_IS_NODE(child));
    g_return_if_fail(self->directory);

    if (file_system_node_is_directory(child)) {
        self->file_count += file_system_node_get_file_count(child);
    } else {
        self->file_count++;
    }
    g_ptr_array_add(self->children, g_object_ref(child));
}

static gint compare_nodes(
    gconstpointer left,
    gconstpointer right
) {
    FileSystemNode *left_node = *(FileSystemNode *const *)left;
    FileSystemNode *right_node = *(FileSystemNode *const *)right;
    gboolean left_directory = file_system_node_is_directory(left_node);
    gboolean right_directory = file_system_node_is_directory(right_node);

    if (left_directory != right_directory) {
        return left_directory ? -1 : 1;
    }
    return g_utf8_collate(
        file_system_node_get_name(left_node),
        file_system_node_get_name(right_node)
    );
}

void file_system_node_sort_children(FileSystemNode *self) {
    g_return_if_fail(FILE_SYSTEM_IS_NODE(self));
    g_ptr_array_sort(self->children, compare_nodes);
}
