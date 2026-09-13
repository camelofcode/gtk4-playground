#ifndef UI_ERROR_DIALOG_H
#define UI_ERROR_DIALOG_H

#include <gtk/gtk.h>

/* Shows a modal error dialog. parent may be NULL; action and error must be
 * non-NULL. The caller retains ownership of all arguments.
 * Returns immediately; the dialog remains visible until dismissed.
 */
void ui_show_error(
    GtkWindow *parent,
    const gchar *action,
    const GError *error
);

#endif
