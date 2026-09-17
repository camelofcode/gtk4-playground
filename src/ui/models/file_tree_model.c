#include "file_tree_model.h"

struct _FileTreeModel {
    GObject parent_instance;
    GListStore *roots;
    GtkTreeListModel *tree;
};

G_DEFINE_TYPE(
    FileTreeModel,
    file_tree_model,
    G_TYPE_OBJECT
)

static GListModel *create_children_model(
    gpointer item,
    gpointer user_data
) {
    (void)user_data;
    FileSystemNode *node = FILE_SYSTEM_NODE(item);
    const GPtrArray *nodes = file_system_node_get_children(node);
    if (!file_system_node_is_directory(node) || nodes->len == 0) {
        return NULL;
    }

    GListStore *children = g_list_store_new(FILE_SYSTEM_TYPE_NODE);
    for (guint i = 0; i < nodes->len; i++) {
        g_list_store_append(children, g_ptr_array_index(nodes, i));
    }
    return G_LIST_MODEL(children);
}

static void file_tree_model_dispose(GObject *object) {
    FileTreeModel *self = FILE_TREE_MODEL(object);
    g_clear_object(&self->tree);
    g_clear_object(&self->roots);
    G_OBJECT_CLASS(file_tree_model_parent_class)->dispose(object);
}

static void file_tree_model_class_init(FileTreeModelClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->dispose = file_tree_model_dispose;
}

static void file_tree_model_init(FileTreeModel *self) {
    self->roots = g_list_store_new(FILE_SYSTEM_TYPE_NODE);
    self->tree = gtk_tree_list_model_new(
        G_LIST_MODEL(g_object_ref(self->roots)),
        FALSE,
        FALSE,
        create_children_model,
        NULL,
        NULL
    );
}

FileTreeModel *file_tree_model_new(void) {
    return g_object_new(FILE_TREE_TYPE_MODEL, NULL);
}

GListModel *file_tree_model_get_model(FileTreeModel *self) {
    g_return_val_if_fail(FILE_TREE_IS_MODEL(self), NULL);
    return G_LIST_MODEL(self->tree);
}

void file_tree_model_set_root(
    FileTreeModel *self,
    FileSystemNode *root
) {
    g_return_if_fail(FILE_TREE_IS_MODEL(self));
    g_return_if_fail(FILE_SYSTEM_IS_NODE(root));
    g_list_store_remove_all(self->roots);
    g_list_store_append(self->roots, root);
}

void file_tree_model_clear(FileTreeModel *self) {
    g_return_if_fail(FILE_TREE_IS_MODEL(self));
    g_list_store_remove_all(self->roots);
}
