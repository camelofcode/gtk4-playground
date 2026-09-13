#include "gtk_helpers.h"

void ui_toggle_css_class(
    GtkWidget *widget,
    const gchar *class_name,
    gboolean enabled
) {
    g_return_if_fail(GTK_IS_WIDGET(widget));
    g_return_if_fail(class_name != NULL);

    if (enabled) {
        gtk_widget_add_css_class(widget, class_name);
    } else {
        gtk_widget_remove_css_class(widget, class_name);
    }
}
