#include "text_binding_window.h"

static void on_text_binding_reset_clicked(
    GtkButton *button,
    GtkEditable *entry
) {
    (void)button;

    gtk_editable_set_text(entry, "");
}

void text_binding_window_init(
    GtkBuilder *builder,
    GtkApplication *app
) {
    GtkWindow *window;
    GtkWindow *main_window;

    window = GTK_WINDOW(gtk_builder_get_object(builder, "text_binding_window"));
    main_window = GTK_WINDOW(gtk_builder_get_object(builder, "main_window"));

    g_object_bind_property(
        gtk_builder_get_object(builder, "text_binding_input"),
        "text",
        gtk_builder_get_object(builder, "text_binding_label"),
        "label",
        G_BINDING_SYNC_CREATE
    );

    g_signal_connect_object(
        gtk_builder_get_object(builder, "text_binding_button"),
        "clicked",
        G_CALLBACK(on_text_binding_reset_clicked),
        gtk_builder_get_object(builder, "text_binding_input"),
        0
    );

    gtk_window_set_application(window, app);

    gtk_window_set_transient_for(window, main_window);
    gtk_window_set_destroy_with_parent(window, TRUE);
}
