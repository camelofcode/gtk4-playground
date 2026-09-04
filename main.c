#include <gtk/gtk.h>

static void
on_button_clicked(GtkButton *button, GtkEntry *entry)
{
    gtk_editable_set_text(GTK_EDITABLE(entry), "");
}

static void
activate(GtkApplication *app, gpointer user_data)
{
    GtkBuilder *builder;
    GtkBuilderScope *scope;
    GtkWindow *window;

    builder = gtk_builder_new();

    scope = gtk_builder_cscope_new();

    gtk_builder_cscope_add_callback_symbol(
        GTK_BUILDER_CSCOPE(scope),
        "on_button_clicked",
        G_CALLBACK(on_button_clicked)
    );

    gtk_builder_set_scope(builder, scope);
    g_object_unref(scope);

    gtk_builder_add_from_file(builder, "main.ui", NULL);

    window = GTK_WINDOW(
        gtk_builder_get_object(builder, "window")
    );

    gtk_window_set_application(window, app);
    gtk_window_present(window);

    g_object_unref(builder);
}

int
main(int argc, char *argv[])
{
    GtkApplication *app;
    int status;

    app = gtk_application_new(
        "com.example.TextExample",
        G_APPLICATION_DEFAULT_FLAGS
    );

    g_signal_connect(
        app,
        "activate",
        G_CALLBACK(activate),
        NULL
    );

    status = g_application_run(
        G_APPLICATION(app),
        argc,
        argv
    );

    g_object_unref(app);

    return status;
}