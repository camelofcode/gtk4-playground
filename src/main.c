#include <gtk/gtk.h>

#include "src/ui/windows/main_window.h"
#include "src/ui/windows/task_list_window.h"
#include "src/ui/windows/text_binding_window.h"

static void load_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();

    gtk_css_provider_load_from_resource(
        provider,
        "/com/camelofdoom/gtk4-playground/ui/app.css"
    );

    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_object_unref(provider);
}

static void activate(
    GtkApplication *app,
    gpointer user_data
) {
    (void)user_data;

    GtkBuilder *builder;
    GError *error = NULL;

    GtkWindow *existing = gtk_application_get_active_window(app);
    if (existing) {
        gtk_window_present(existing);
        return;
    }

    load_css();

    builder = gtk_builder_new();
    const char *window_resources[] = {
        "/com/camelofdoom/gtk4-playground/ui/windows/main-window.ui",
        "/com/camelofdoom/gtk4-playground/ui/windows/text-binding-window.ui",
        "/com/camelofdoom/gtk4-playground/ui/windows/task-list-window.ui",
    };
    for (guint i = 0; i < G_N_ELEMENTS(window_resources); i++) {
        if (!gtk_builder_add_from_resource(builder, window_resources[i], &error)) {
            g_error("Failed to load UI %s: %s", window_resources[i], error->message);
        }
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
        "com.example.Gtk4Playground",
        G_APPLICATION_DEFAULT_FLAGS
    );

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);

    return status;
}
