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

void remove_item_from_list(GListStore *list, gpointer item) {
    g_return_if_fail(G_IS_LIST_STORE(list));
    g_return_if_fail(G_IS_OBJECT(item));

    GListModel *model = G_LIST_MODEL(list);
    for (guint i = 0; i < g_list_model_get_n_items(model); i++) {
        g_autoptr(GObject) current = g_list_model_get_item(model, i);
        if (current == item) {
            g_list_store_remove(list, i);
            return;
        }
    }
}
