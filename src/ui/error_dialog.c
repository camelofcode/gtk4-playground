#include "error_dialog.h"

void ui_show_error(
    GtkWindow *parent,
    const gchar *action,
    const GError *error
) {
    g_autoptr(GtkAlertDialog) dialog = gtk_alert_dialog_new("%s", action);
    gtk_alert_dialog_set_detail(dialog, error->message);
    gtk_alert_dialog_show(dialog, parent);
}
