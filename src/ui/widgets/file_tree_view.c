#include "file_tree_view.h"
#include "src/core/file_system_loader.h"
#include "src/ui/models/file_tree_model.h"

enum {
    PROP_0,
    PROP_LOADING,
    PROP_STATUS_MESSAGE,
    N_PROPERTIES,
};

typedef enum {
    TEXT_COLUMN_SIZE,
    TEXT_COLUMN_FILE_COUNT,
    TEXT_COLUMN_MODIFIED,
} TextColumn;

struct _FileTreeView {
    GtkBox parent_instance;

    GtkColumnView *column_view;
    FileTreeModel *model;
    GtkSingleSelection *selection;
    GCancellable *cancellable;
    gchar *status_message;
    gboolean loading;
};

G_DEFINE_TYPE(
    FileTreeView,
    file_tree_view,
    GTK_TYPE_BOX
)

static GParamSpec *properties[N_PROPERTIES];

static void file_tree_view_get_property(
    GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec
) {
    FileTreeView *self = FILE_TREE_VIEW(object);

    switch (property_id) {
    case PROP_LOADING:
        g_value_set_boolean(value, self->loading);
        break;
    case PROP_STATUS_MESSAGE:
        g_value_set_string(value, self->status_message);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void file_tree_view_set_loading(
    FileTreeView *self,
    gboolean loading,
    const gchar *message
) {
    if (g_strcmp0(self->status_message, message) != 0) {
        g_free(self->status_message);
        self->status_message = g_strdup(message);
        g_object_notify_by_pspec(
            G_OBJECT(self),
            properties[PROP_STATUS_MESSAGE]
        );
    }

    if (self->loading != loading) {
        self->loading = loading;
        g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_LOADING]);
    }
}

static FileSystemNode *get_list_item_node(GtkListItem *list_item) {
    GtkTreeListRow *row = GTK_TREE_LIST_ROW(
        gtk_list_item_get_item(list_item)
    );
    return FILE_SYSTEM_NODE(gtk_tree_list_row_get_item(row));
}

static gchar *format_modified_time(gint64 modified_time) {
    if (modified_time <= 0) {
        return g_strdup("");
    }

    g_autoptr(GDateTime) date_time =
        g_date_time_new_from_unix_local(modified_time);
    return date_time != NULL
        ? g_date_time_format(date_time, "%Y-%m-%d %H:%M")
        : g_strdup("");
}

static void setup_name_cell(
    GtkSignalListItemFactory *factory,
    GtkListItem *list_item,
    gpointer user_data
) {
    (void)factory;
    (void)user_data;
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *image = gtk_image_new();
    GtkWidget *label = gtk_label_new(NULL);
    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
    gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
    gtk_widget_set_hexpand(label, TRUE);
    gtk_box_append(GTK_BOX(box), image);
    gtk_box_append(GTK_BOX(box), label);

    GtkWidget *expander = gtk_tree_expander_new();
    gtk_tree_expander_set_child(GTK_TREE_EXPANDER(expander), box);
    gtk_list_item_set_focusable(list_item, FALSE);
    gtk_list_item_set_child(list_item, expander);
}

static void bind_name_cell(
    GtkSignalListItemFactory *factory,
    GtkListItem *list_item,
    gpointer user_data
) {
    (void)factory;
    (void)user_data;
    GtkTreeListRow *row = GTK_TREE_LIST_ROW(
        gtk_list_item_get_item(list_item)
    );
    FileSystemNode *node = FILE_SYSTEM_NODE(
        gtk_tree_list_row_get_item(row)
    );
    GtkTreeExpander *expander = GTK_TREE_EXPANDER(
        gtk_list_item_get_child(list_item)
    );
    GtkWidget *box = gtk_tree_expander_get_child(expander);
    GtkImage *image = GTK_IMAGE(gtk_widget_get_first_child(box));
    GtkLabel *label = GTK_LABEL(
        gtk_widget_get_next_sibling(GTK_WIDGET(image))
    );

    g_autoptr(GIcon) icon = g_themed_icon_new_with_default_fallbacks(
        file_system_node_is_directory(node) ? "folder" : "text-x-generic"
    );
    gtk_tree_expander_set_list_row(expander, row);
    gtk_image_set_from_gicon(image, icon);
    gtk_label_set_text(label, file_system_node_get_name(node));
}

static void unbind_name_cell(
    GtkSignalListItemFactory *factory,
    GtkListItem *list_item,
    gpointer user_data
) {
    (void)factory;
    (void)user_data;
    GtkTreeExpander *expander = GTK_TREE_EXPANDER(
        gtk_list_item_get_child(list_item)
    );
    GtkWidget *box = gtk_tree_expander_get_child(expander);
    GtkImage *image = GTK_IMAGE(gtk_widget_get_first_child(box));
    GtkLabel *label = GTK_LABEL(
        gtk_widget_get_next_sibling(GTK_WIDGET(image))
    );
    gtk_tree_expander_set_list_row(expander, NULL);
    gtk_image_clear(image);
    gtk_label_set_text(label, "");
}

static void setup_text_cell(
    GtkSignalListItemFactory *factory,
    GtkListItem *list_item,
    gpointer user_data
) {
    (void)factory;
    TextColumn column = GPOINTER_TO_INT(user_data);
    GtkWidget *label = gtk_label_new(NULL);
    gtk_label_set_xalign(
        GTK_LABEL(label),
        column == TEXT_COLUMN_MODIFIED ? 0.0f : 1.0f
    );
    gtk_list_item_set_child(list_item, label);
}

static void bind_text_cell(
    GtkSignalListItemFactory *factory,
    GtkListItem *list_item,
    gpointer user_data
) {
    (void)factory;
    TextColumn column = GPOINTER_TO_INT(user_data);
    FileSystemNode *node = get_list_item_node(list_item);
    GtkLabel *label = GTK_LABEL(gtk_list_item_get_child(list_item));
    g_autofree gchar *text = NULL;

    switch (column) {
    case TEXT_COLUMN_SIZE:
        text = file_system_node_is_directory(node)
            ? g_strdup("")
            : g_format_size_full(
                file_system_node_get_size(node),
                G_FORMAT_SIZE_IEC_UNITS
            );
        break;
    case TEXT_COLUMN_FILE_COUNT:
        text = file_system_node_is_directory(node)
            ? g_strdup_printf(
                "%" G_GUINT64_FORMAT,
                file_system_node_get_file_count(node)
            )
            : g_strdup("");
        break;
    case TEXT_COLUMN_MODIFIED:
        text = format_modified_time(
            file_system_node_get_modified_time(node)
        );
        break;
    }

    gtk_label_set_text(label, text);
}

static GtkColumnViewColumn *create_name_column(void) {
    GtkListItemFactory *factory = gtk_signal_list_item_factory_new();
    g_signal_connect(factory, "setup", G_CALLBACK(setup_name_cell), NULL);
    g_signal_connect(factory, "bind", G_CALLBACK(bind_name_cell), NULL);
    g_signal_connect(factory, "unbind", G_CALLBACK(unbind_name_cell), NULL);
    GtkColumnViewColumn *column = gtk_column_view_column_new("Name", factory);
    gtk_column_view_column_set_expand(column, TRUE);
    gtk_column_view_column_set_resizable(column, TRUE);
    return column;
}

static GtkColumnViewColumn *create_text_column(
    const gchar *title,
    TextColumn text_column
) {
    GtkListItemFactory *factory = gtk_signal_list_item_factory_new();
    g_signal_connect(
        factory,
        "setup",
        G_CALLBACK(setup_text_cell),
        GINT_TO_POINTER(text_column)
    );
    g_signal_connect(
        factory,
        "bind",
        G_CALLBACK(bind_text_cell),
        GINT_TO_POINTER(text_column)
    );
    GtkColumnViewColumn *column = gtk_column_view_column_new(title, factory);
    gtk_column_view_column_set_resizable(column, TRUE);
    return column;
}

static void append_column(
    GtkColumnView *view,
    GtkColumnViewColumn *column
) {
    gtk_column_view_append_column(view, column);
    g_object_unref(column);
}

static void configure_column_view(FileTreeView *self) {
    append_column(self->column_view, create_name_column());
    append_column(
        self->column_view,
        create_text_column("Size", TEXT_COLUMN_SIZE)
    );
    append_column(
        self->column_view,
        create_text_column("Files", TEXT_COLUMN_FILE_COUNT)
    );
    append_column(
        self->column_view,
        create_text_column("Modified", TEXT_COLUMN_MODIFIED)
    );

    self->selection = gtk_single_selection_new(
        g_object_ref(file_tree_model_get_model(self->model))
    );
    gtk_single_selection_set_autoselect(self->selection, FALSE);
    gtk_single_selection_set_can_unselect(self->selection, TRUE);
    gtk_column_view_set_model(
        self->column_view,
        GTK_SELECTION_MODEL(self->selection)
    );
}

static void on_folder_loaded(
    GObject *source_object,
    GAsyncResult *result,
    gpointer user_data
) {
    (void)source_object;
    FileTreeView *self = FILE_TREE_VIEW(user_data);
    g_autoptr(GError) error = NULL;
    g_autoptr(FileSystemNode) root = file_system_loader_load_finish(
        result,
        &error
    );
    g_clear_object(&self->cancellable);

    if (root == NULL) {
        if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
            file_tree_view_set_loading(self, FALSE, "Loading cancelled");
        } else {
            g_autofree gchar *message = g_strdup_printf(
                "Could not load folder: %s",
                error->message
            );
            file_tree_view_set_loading(self, FALSE, message);
        }
        g_object_unref(self);
        return;
    }

    file_tree_model_set_root(self->model, root);
    g_autoptr(GtkTreeListRow) root_row = gtk_tree_list_model_get_child_row(
        GTK_TREE_LIST_MODEL(file_tree_model_get_model(self->model)),
        0
    );
    if (root_row != NULL) {
        gtk_tree_list_row_set_expanded(root_row, TRUE);
    }

    guint64 file_count = file_system_node_get_file_count(root);
    g_autofree gchar *message = g_strdup_printf(
        "Loaded %" G_GUINT64_FORMAT " file%s",
        file_count,
        file_count == 1 ? "" : "s"
    );
    file_tree_view_set_loading(self, FALSE, message);
    g_object_unref(self);
}

static void file_tree_view_dispose(GObject *object) {
    FileTreeView *self = FILE_TREE_VIEW(object);
    if (self->cancellable != NULL) {
        g_cancellable_cancel(self->cancellable);
    }
    if (self->column_view != NULL) {
        gtk_column_view_set_model(self->column_view, NULL);
    }
    g_clear_object(&self->cancellable);
    g_clear_object(&self->selection);
    g_clear_object(&self->model);
    gtk_widget_dispose_template(GTK_WIDGET(self), FILE_TREE_TYPE_VIEW);
    G_OBJECT_CLASS(file_tree_view_parent_class)->dispose(object);
}

static void file_tree_view_finalize(GObject *object) {
    FileTreeView *self = FILE_TREE_VIEW(object);
    g_clear_pointer(&self->status_message, g_free);
    G_OBJECT_CLASS(file_tree_view_parent_class)->finalize(object);
}

static void file_tree_view_class_init(FileTreeViewClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    object_class->get_property = file_tree_view_get_property;
    object_class->dispose = file_tree_view_dispose;
    object_class->finalize = file_tree_view_finalize;

    properties[PROP_LOADING] = g_param_spec_boolean(
        "loading",
        "Loading",
        "Whether the filesystem is being loaded",
        FALSE,
        G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY
    );
    properties[PROP_STATUS_MESSAGE] = g_param_spec_string(
        "status-message",
        "Status message",
        "A human-readable description of the current loading state",
        "Choose a folder to load its contents",
        G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY
    );
    g_object_class_install_properties(object_class, N_PROPERTIES, properties);

    gtk_widget_class_set_template_from_resource(
        widget_class,
        "/com/camelofdoom/gtk4-playground/ui/widgets/file-tree-view.ui"
    );
    gtk_widget_class_bind_template_child(
        widget_class,
        FileTreeView,
        column_view
    );
}

static void file_tree_view_init(FileTreeView *self) {
    self->status_message = g_strdup("Choose a folder to load its contents");
    gtk_widget_init_template(GTK_WIDGET(self));
    self->model = file_tree_model_new();
    configure_column_view(self);
}

FileTreeView *file_tree_view_new(void) {
    return g_object_new(FILE_TREE_TYPE_VIEW, NULL);
}

void file_tree_view_load_path(
    FileTreeView *self,
    const gchar *path
) {
    g_return_if_fail(FILE_TREE_IS_VIEW(self));
    g_return_if_fail(path != NULL);
    g_return_if_fail(!self->loading);

    file_tree_model_clear(self->model);
    self->cancellable = g_cancellable_new();
    file_tree_view_set_loading(self, TRUE, "Loading folder…");
    file_system_loader_load_async(
        path,
        self->cancellable,
        on_folder_loaded,
        g_object_ref(self)
    );
}

gboolean file_tree_view_get_loading(FileTreeView *self) {
    g_return_val_if_fail(FILE_TREE_IS_VIEW(self), FALSE);
    return self->loading;
}

const gchar *file_tree_view_get_status_message(FileTreeView *self) {
    g_return_val_if_fail(FILE_TREE_IS_VIEW(self), NULL);
    return self->status_message;
}
