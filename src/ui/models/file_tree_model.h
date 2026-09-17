#ifndef FILE_TREE_MODEL_H
#define FILE_TREE_MODEL_H

#include "src/core/file_system_node.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define FILE_TREE_TYPE_MODEL file_tree_model_get_type()

G_DECLARE_FINAL_TYPE(
    FileTreeModel,
    file_tree_model,
    FILE_TREE,
    MODEL,
    GObject
)

FileTreeModel *file_tree_model_new(void);
GListModel *file_tree_model_get_model(FileTreeModel *self);
void file_tree_model_set_root(
    FileTreeModel *self,
    FileSystemNode *root
);
void file_tree_model_clear(FileTreeModel *self);

G_END_DECLS

#endif
