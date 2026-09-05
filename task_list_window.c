#include "task_list_window.h"

void task_list_window_register_callbacks(GtkBuilderCScope *scope) {}

void task_list_window_init(
    GtkBuilder *builder,
    GtkApplication *app
) {
    GtkWindow *window;
    GtkWindow *main_window;

    window = GTK_WINDOW(gtk_builder_get_object(builder, "task_list_window"));
    main_window = GTK_WINDOW(gtk_builder_get_object(builder, "main_window"));

    gtk_window_set_application(window, app);

    gtk_window_set_transient_for(window, main_window);
    gtk_window_set_destroy_with_parent(window, TRUE);
}