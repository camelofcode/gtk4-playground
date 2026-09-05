#include "main_window.h"

static void
on_open_text_binding_window_button_clicked(
    GtkButton *button,
    GtkWindow *text_binding_window)
{
    gtk_window_present(text_binding_window);
}

static void
on_open_task_list_window_button_clicked(
    GtkButton *button,
    GtkWindow *task_list_window)
{
    gtk_window_present(task_list_window);
}

void
main_window_register_callbacks(GtkBuilderCScope *scope)
{
    gtk_builder_cscope_add_callback_symbol(
        scope,
        "on_open_text_binding_window_button_clicked",
        G_CALLBACK(on_open_text_binding_window_button_clicked)
    );

    gtk_builder_cscope_add_callback_symbol(
        scope,
        "on_open_task_list_window_button_clicked",
        G_CALLBACK(on_open_task_list_window_button_clicked)
    );
}

void
main_window_init(
    GtkBuilder *builder,
    GtkApplication *app)
{
    GtkWindow *window;

    window = GTK_WINDOW(
        gtk_builder_get_object(builder, "main_window")
    );

    gtk_window_set_application(window, app);
    gtk_window_present(window);
}