#ifndef FILE_TREE_VIEW_H
#define FILE_TREE_VIEW_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define FILE_TREE_TYPE_VIEW file_tree_view_get_type()

G_DECLARE_FINAL_TYPE(
    FileTreeView,
    file_tree_view,
    FILE_TREE,
    VIEW,
    GtkBox
)

FileTreeView *file_tree_view_new(void);
void file_tree_view_load_path(
    FileTreeView *self,
    const gchar *path
);
gboolean file_tree_view_get_loading(FileTreeView *self);
const gchar *file_tree_view_get_status_message(FileTreeView *self);

G_END_DECLS

#endif
