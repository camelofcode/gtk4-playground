#include "file_tree_window.h"
#include "src/ui/widgets/file_tree_view.h"

static const char FILE_TREE_WINDOW_STATE_KEY[] = "file-tree-window-state";

typedef struct {
    GtkWindow *window;
    GtkWidget *folder_chooser_box;
    GtkEntry *folder_path_entry;
    GtkSpinner *spinner;
    GtkLabel *status_label;
    FileTreeView *file_tree_view;
} FileTreeWindowState;

static void on_file_tree_view_status_changed(
    FileTreeView *file_tree_view,
    GParamSpec *pspec,
    gpointer user_data
) {
    FileTreeWindowState *state = user_data;
    gboolean loading = file_tree_view_get_loading(file_tree_view);
    gtk_widget_set_sensitive(state->folder_chooser_box, !loading);
    gtk_label_set_text(
        state->status_label,
        file_tree_view_get_status_message(file_tree_view)
    );
    gtk_widget_set_visible(GTK_WIDGET(state->spinner), loading);
    if (loading) {
        gtk_spinner_start(state->spinner);
    } else {
        gtk_spinner_stop(state->spinner);
    }
    (void)pspec;
}

static void on_folder_selected(
    GObject *source_object,
    GAsyncResult *result,
    gpointer user_data
) {
    GtkWindow *window = GTK_WINDOW(user_data);
    FileTreeWindowState *state = g_object_get_data(
        G_OBJECT(window),
        FILE_TREE_WINDOW_STATE_KEY
    );
    g_autoptr(GError) error = NULL;
    g_autoptr(GFile) folder = gtk_file_dialog_select_folder_finish(
        GTK_FILE_DIALOG(source_object),
        result,
        &error
    );

    if (folder != NULL) {
        g_autofree gchar *path = g_file_get_path(folder);
        if (path != NULL) {
            gtk_editable_set_text(GTK_EDITABLE(state->folder_path_entry), path);
            gtk_editable_set_position(
                GTK_EDITABLE(state->folder_path_entry),
                -1
            );
            file_tree_view_load_path(state->file_tree_view, path);
        }
    }

    g_object_unref(window);
}

static void on_folder_browse_button_clicked(
    GtkButton *button,
    gpointer user_data
) {
    FileTreeWindowState *state = user_data;
    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, "Select Folder");
    gtk_file_dialog_set_accept_label(dialog, "Select");

    const gchar *current_path = gtk_editable_get_text(
        GTK_EDITABLE(state->folder_path_entry)
    );
    if (current_path[0] != '\0') {
        g_autoptr(GFile) current_folder = g_file_new_for_path(current_path);
        gtk_file_dialog_set_initial_folder(dialog, current_folder);
    }

    gtk_file_dialog_select_folder(
        dialog,
        state->window,
        NULL,
        on_folder_selected,
        g_object_ref(state->window)
    );
    g_object_unref(dialog);
    (void)button;
}

void file_tree_window_init(
    GtkBuilder *builder,
    GtkApplication *app
) {
    FileTreeWindowState *state = g_new0(FileTreeWindowState, 1);
    state->window = GTK_WINDOW(
        gtk_builder_get_object(builder, "file_tree_window")
    );
    state->folder_chooser_box = GTK_WIDGET(
        gtk_builder_get_object(builder, "folder_chooser_box")
    );
    state->folder_path_entry = GTK_ENTRY(
        gtk_builder_get_object(builder, "folder_path_entry")
    );
    state->spinner = GTK_SPINNER(
        gtk_builder_get_object(builder, "file_tree_spinner")
    );
    state->status_label = GTK_LABEL(
        gtk_builder_get_object(builder, "file_tree_status_label")
    );
    state->file_tree_view = file_tree_view_new();

    GtkBox *container = GTK_BOX(
        gtk_builder_get_object(builder, "file_tree_view_container")
    );
    gtk_box_append(container, GTK_WIDGET(state->file_tree_view));

    g_object_set_data_full(
        G_OBJECT(state->window),
        FILE_TREE_WINDOW_STATE_KEY,
        state,
        g_free
    );
    g_signal_connect(
        state->file_tree_view,
        "notify::loading",
        G_CALLBACK(on_file_tree_view_status_changed),
        state
    );
    g_signal_connect(
        state->file_tree_view,
        "notify::status-message",
        G_CALLBACK(on_file_tree_view_status_changed),
        state
    );
    g_signal_connect(
        gtk_builder_get_object(builder, "folder_browse_button"),
        "clicked",
        G_CALLBACK(on_folder_browse_button_clicked),
        state
    );

    GtkWindow *main_window = GTK_WINDOW(
        gtk_builder_get_object(builder, "main_window")
    );
    gtk_window_set_application(state->window, app);
    gtk_window_set_transient_for(state->window, main_window);
    gtk_window_set_destroy_with_parent(state->window, TRUE);
}
