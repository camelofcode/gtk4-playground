#include "text_binding_window.h"

static void on_text_binding_reset_clicked(
    GtkButton *button,
    GtkEditable *entry
) {
    gtk_editable_set_text(entry, "");
}

void text_binding_window_register_callbacks(GtkBuilderCScope *scope) {
    gtk_builder_cscope_add_callback_symbol(
        scope,
        "on_text_binding_reset_clicked",
        G_CALLBACK(on_text_binding_reset_clicked)
    );
}

void text_binding_window_init(
    GtkBuilder *builder,
    GtkApplication *app
) {
    GtkWindow *window;
    GtkWindow *main_window;

    window = GTK_WINDOW(gtk_builder_get_object(builder, "text_binding_window"));
    main_window = GTK_WINDOW(gtk_builder_get_object(builder, "main_window"));

    gtk_window_set_application(window, app);

    gtk_window_set_transient_for(window, main_window);
    gtk_window_set_destroy_with_parent(window, TRUE);
}