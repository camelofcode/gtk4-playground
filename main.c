#include <gtk/gtk.h>

#include "main_window.h"
#include "task_list_window.h"
#include "text_binding_window.h"

static void activate(
    GtkApplication *app,
    gpointer user_data
) {
    GtkBuilder *builder;
    GtkBuilderScope *scope;
    GError *error = NULL;

    builder = gtk_builder_new();
    scope = gtk_builder_cscope_new();

    /*
     * Each module registers the callback functions that its
     * part of main.ui refers to.
     */
    main_window_register_callbacks(GTK_BUILDER_CSCOPE(scope));
    text_binding_window_register_callbacks(GTK_BUILDER_CSCOPE(scope));
    task_list_window_register_callbacks(GTK_BUILDER_CSCOPE(scope));

    gtk_builder_set_scope(builder, GTK_BUILDER_SCOPE(scope));

    g_object_unref(scope);

    if (!gtk_builder_add_from_resource(
            builder,
            "/com/camelofdoom/gtk-playground/main.ui",
            &error
        )) {
        g_error("Failed to load UI: %s", error->message);
    }

    /*
     * Let each module initialise its own window.
     */
    text_binding_window_init(builder, app);
    main_window_init(builder, app);
    task_list_window_init(builder, app);

    g_object_unref(builder);
}

int main(
    int argc,
    char *argv[]
) {
    GtkApplication *app;
    int status;

    app = gtk_application_new(
        "com.example.GtkPlayground",
        G_APPLICATION_DEFAULT_FLAGS
    );

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);

    return status;
}