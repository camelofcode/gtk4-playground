#include "src/core/task_storage.h"
#include "src/ui/widgets/task_row.h"
#include "src/ui/windows/main_window.h"
#include "src/ui/windows/task_list_window.h"
#include "src/ui/windows/text_binding_window.h"
#include <glib/gstdio.h>
#include <gtk/gtk.h>

static guint run_number;

static void activate_test(
    GtkApplication *app,
    gpointer user_data
) {
    (void)user_data;
    g_autoptr(GtkBuilder) builder = gtk_builder_new();
    const char *window_resources[] = {
        "/com/camelofdoom/gtk4-playground/ui/windows/main-window.ui",
        "/com/camelofdoom/gtk4-playground/ui/windows/text-binding-window.ui",
        "/com/camelofdoom/gtk4-playground/ui/windows/task-list-window.ui",
    };
    for (guint i = 0; i < G_N_ELEMENTS(window_resources); i++) {
        g_autoptr(GError) build_error = NULL;
        g_assert_true(gtk_builder_add_from_resource(
            builder,
            window_resources[i],
            &build_error
        ));
        g_assert_no_error(build_error);
    }
    text_binding_window_init(builder, app);
    main_window_init(builder, app);
    task_list_window_init(builder, app);
    GtkWidget *main_window =
        GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    GtkWidget *window =
        GTK_WIDGET(gtk_builder_get_object(builder, "task_list_window"));
    GtkEntry *input = GTK_ENTRY(gtk_builder_get_object(builder, "task_input"));
    GtkListBox *list =
        GTK_LIST_BOX(gtk_builder_get_object(builder, "task_list"));
    gtk_window_present(GTK_WINDOW(window));

    /* Exercise the connections established by each window initializer. */
    GtkWidget *text_window =
        GTK_WIDGET(gtk_builder_get_object(builder, "text_binding_window"));
    g_signal_emit_by_name(
        gtk_builder_get_object(builder, "open_text_binding_window_button"),
        "clicked"
    );
    g_assert_true(gtk_widget_get_visible(text_window));
    GtkEntry *text_input =
        GTK_ENTRY(gtk_builder_get_object(builder, "text_binding_input"));
    gtk_editable_set_text(GTK_EDITABLE(text_input), "Reset me");
    g_signal_emit_by_name(
        gtk_builder_get_object(builder, "text_binding_button"),
        "clicked"
    );
    g_assert_cmpstr(gtk_editable_get_text(GTK_EDITABLE(text_input)), ==, "");
    gtk_window_close(GTK_WINDOW(text_window));
    g_assert_false(gtk_widget_get_visible(text_window));
    gtk_widget_set_visible(window, FALSE);
    g_signal_emit_by_name(
        gtk_builder_get_object(builder, "open_task_list_window_button"),
        "clicked"
    );
    g_assert_true(gtk_widget_get_visible(window));

    if (run_number == 0) {
        gtk_editable_set_text(GTK_EDITABLE(input), "Persist from the UI");
        g_signal_emit_by_name(input, "activate");
        TaskRow *row = TASK_ROW(gtk_list_box_get_row_at_index(list, 0));
        g_assert_nonnull(row);
        GtkToggleButton *toggle =
            GTK_TOGGLE_BUTTON(gtk_widget_get_template_child(
                GTK_WIDGET(row),
                TASK_TYPE_ROW,
                "done_button"
            ));
        gtk_toggle_button_set_active(toggle, TRUE);
        g_assert_true(task_get_done(task_row_get_task(row)));
        gtk_window_close(GTK_WINDOW(window));
        g_assert_false(gtk_widget_get_visible(window));
        g_autofree gchar *path = task_storage_default_path();
        g_autoptr(GError) error = NULL;
        g_autoptr(GListStore) saved = task_storage_load(path, &error);
        g_assert_no_error(error);
        g_assert_cmpuint(g_list_model_get_n_items(G_LIST_MODEL(saved)), ==, 1);
        gtk_window_present(GTK_WINDOW(window));
        g_assert_true(task_get_done(task_row_get_task(row)));
        /* A later edit must survive closing only the main window. */
        gtk_editable_set_text(GTK_EDITABLE(input), "Saved during shutdown");
        g_signal_emit_by_name(input, "activate");
    } else {
        TaskRow *first = TASK_ROW(gtk_list_box_get_row_at_index(list, 0));
        TaskRow *second = TASK_ROW(gtk_list_box_get_row_at_index(list, 1));
        g_assert_nonnull(first);
        g_assert_nonnull(second);
        g_assert_cmpstr(
            task_get_text(task_row_get_task(first)),
            ==,
            "Persist from the UI"
        );
        g_assert_true(task_get_done(task_row_get_task(first)));
        GtkToggleButton *toggle =
            GTK_TOGGLE_BUTTON(gtk_widget_get_template_child(
                GTK_WIDGET(first),
                TASK_TYPE_ROW,
                "done_button"
            ));
        g_assert_true(gtk_toggle_button_get_active(toggle));
        g_assert_cmpstr(
            task_get_text(task_row_get_task(second)),
            ==,
            "Saved during shutdown"
        );
        GObject *delete_button = gtk_widget_get_template_child(
            GTK_WIDGET(first),
            TASK_TYPE_ROW,
            "delete_button"
        );
        g_signal_emit_by_name(delete_button, "clicked");
        g_assert_null(gtk_list_box_get_row_at_index(list, 1));
    }
    gtk_window_close(GTK_WINDOW(main_window));
}

int main(
    int argc,
    char **argv
) {
    (void)argc;
    (void)argv;
    g_autofree gchar *directory =
        g_dir_make_tmp("gtk4-window-test-XXXXXX", NULL);
    g_setenv("XDG_DATA_HOME", directory, TRUE);
    if (!gtk_init_check()) {
        g_rmdir(directory);
        return 77;
    }
    for (run_number = 0; run_number < 2; run_number++) {
        g_autoptr(GtkApplication) app = gtk_application_new(
            "com.example.Gtk4Playground.Test",
            G_APPLICATION_NON_UNIQUE
        );
        g_signal_connect(app, "activate", G_CALLBACK(activate_test), NULL);
        g_assert_cmpint(g_application_run(G_APPLICATION(app), 0, NULL), ==, 0);
    }
    g_autofree gchar *path = task_storage_default_path();
    g_autoptr(GError) error = NULL;
    g_autoptr(GListStore) saved = task_storage_load(path, &error);
    g_assert_no_error(error);
    g_assert_cmpuint(g_list_model_get_n_items(G_LIST_MODEL(saved)), ==, 1);
    g_autoptr(Task) remaining = g_list_model_get_item(G_LIST_MODEL(saved), 0);
    g_assert_cmpstr(task_get_text(remaining), ==, "Saved during shutdown");
    g_remove(path);
    g_autofree gchar *subdir = g_path_get_dirname(path);
    g_rmdir(subdir);
    g_rmdir(directory);
    return 0;
}
