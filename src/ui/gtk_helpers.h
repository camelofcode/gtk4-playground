#ifndef UI_GTK_HELPERS_H
#define UI_GTK_HELPERS_H

#include <gtk/gtk.h>

/* Ensures class_name is present when enabled, absent otherwise. */
void ui_toggle_css_class(
    GtkWidget *widget,
    const gchar *class_name,
    gboolean enabled
);

#endif
