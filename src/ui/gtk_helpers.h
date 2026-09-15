#ifndef UI_GTK_HELPERS_H
#define UI_GTK_HELPERS_H

#include <gtk/gtk.h>

/* Ensures class_name is present when enabled, absent otherwise. */
void ui_toggle_css_class(
    GtkWidget *widget,
    const gchar *class_name,
    gboolean enabled
);

/* Removes the first pointer-identical item, if present. Borrows item. */
void remove_item_from_list(GListStore *list, gpointer item);

#endif
