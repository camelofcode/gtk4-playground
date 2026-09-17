#include "src/ui/widgets/file_tree_view.h"
#include <glib/gstdio.h>

typedef struct {
    GMainLoop *loop;
    gboolean completed;
} LoadState;

static gchar *create_test_tree(void) {
    g_autoptr(GError) error = NULL;
    gchar *root = g_dir_make_tmp("file-tree-view-XXXXXX", &error);
    g_assert_no_error(error);
    g_autofree gchar *folder = g_build_filename(root, "folder", NULL);
    g_autofree gchar *file = g_build_filename(root, "file.txt", NULL);
    g_assert_cmpint(g_mkdir(folder, 0700), ==, 0);
    g_assert_true(g_file_set_contents(file, "content", 7, &error));
    g_assert_no_error(error);
    return root;
}

static void remove_test_tree(const gchar *root) {
    g_autofree gchar *folder = g_build_filename(root, "folder", NULL);
    g_autofree gchar *file = g_build_filename(root, "file.txt", NULL);
    g_remove(file);
    g_rmdir(folder);
    g_rmdir(root);
}

static void on_loading_changed(
    FileTreeView *view,
    GParamSpec *pspec,
    gpointer user_data
) {
    (void)pspec;
    LoadState *state = user_data;
    if (!file_tree_view_get_loading(view)) {
        state->completed = TRUE;
        g_main_loop_quit(state->loop);
    }
}

int main(
    int argc,
    char **argv
) {
    (void)argc;
    (void)argv;
    if (!gtk_init_check()) {
        return 77;
    }

    g_autofree gchar *root = create_test_tree();
    GtkWidget *window = gtk_window_new();
    FileTreeView *view = file_tree_view_new();
    gtk_window_set_child(GTK_WINDOW(window), GTK_WIDGET(view));
    gtk_window_present(GTK_WINDOW(window));

    LoadState state = {
        .loop = g_main_loop_new(NULL, FALSE),
    };
    g_signal_connect(
        view,
        "notify::loading",
        G_CALLBACK(on_loading_changed),
        &state
    );
    file_tree_view_load_path(view, root);
    g_main_loop_run(state.loop);

    g_assert_true(state.completed);
    g_assert_false(file_tree_view_get_loading(view));
    g_assert_true(g_str_has_prefix(
        file_tree_view_get_status_message(view),
        "Loaded 1 file"
    ));

    GtkColumnView *column_view = GTK_COLUMN_VIEW(
        gtk_widget_get_template_child(
            GTK_WIDGET(view),
            FILE_TREE_TYPE_VIEW,
            "column_view"
        )
    );
    GtkSelectionModel *selection = gtk_column_view_get_model(column_view);
    g_assert_cmpuint(
        g_list_model_get_n_items(G_LIST_MODEL(selection)),
        ==,
        3
    );

    while (g_main_context_pending(NULL)) {
        g_main_context_iteration(NULL, FALSE);
    }

    g_main_loop_unref(state.loop);
    gtk_window_destroy(GTK_WINDOW(window));
    remove_test_tree(root);
    return 0;
}
