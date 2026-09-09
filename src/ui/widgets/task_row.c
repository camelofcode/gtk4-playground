#include "task_row.h"
#include "gtk/gtk.h"
#include "gtk/gtkshortcut.h"

enum {
    PROP_0,
    PROP_TEXT,
    PROP_DONE,
    N_PROPERTIES,
};

enum {
    DELETE_REQUESTED,
    N_SIGNALS,
};

struct _TaskRow {
    GtkListBoxRow parent_instance;

    GtkLabel *text_label;
    GtkToggleButton *done_button;
    GtkImage *done_image;

    gchar *text;
    gboolean done;
};

G_DEFINE_TYPE(
    TaskRow,
    task_row,
    GTK_TYPE_LIST_BOX_ROW
)

static GParamSpec *properties[N_PROPERTIES];
static guint signals[N_SIGNALS];

static void task_row_update_appearance(TaskRow *self) {
    PangoAttrList *attributes = NULL;

    gtk_label_set_text(self->text_label, self->text);

    char *complete_class = "complete";

    if (self->done) {
        attributes = pango_attr_list_new();
        pango_attr_list_insert(attributes, pango_attr_strikethrough_new(TRUE));

        if (!gtk_widget_has_css_class((GtkWidget *)self, complete_class))
            gtk_widget_add_css_class((GtkWidget *)self, complete_class);
    } else {
        if (gtk_widget_has_css_class((GtkWidget *)self, complete_class))
            gtk_widget_remove_css_class((GtkWidget *)self, complete_class);
    }

    gtk_label_set_attributes(self->text_label, attributes);

    if (attributes != NULL) {
        pango_attr_list_unref(attributes);
    }

    gtk_image_set_from_icon_name(
        self->done_image,
        self->done ? "edit-undo-symbolic" : "object-select-symbolic"
    );
    gtk_widget_set_tooltip_text(
        GTK_WIDGET(self->done_button),
        self->done ? "Mark task as not done" : "Mark task as done"
    );
}

static void task_row_on_done_toggled(
    GtkToggleButton *button,
    TaskRow *self
) {
    task_row_set_done(self, gtk_toggle_button_get_active(button));
}

static void task_row_on_delete_clicked(
    GtkButton *button,
    TaskRow *self
) {
    (void)button;

    g_signal_emit(self, signals[DELETE_REQUESTED], 0);
}

static void task_row_get_property(
    GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec
) {
    TaskRow *self = TASK_ROW(object);

    switch (property_id) {
    case PROP_TEXT:
        g_value_set_string(value, self->text);
        break;
    case PROP_DONE:
        g_value_set_boolean(value, self->done);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void task_row_set_property(
    GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec
) {
    TaskRow *self = TASK_ROW(object);

    switch (property_id) {
    case PROP_TEXT:
        task_row_set_text(self, g_value_get_string(value));
        break;
    case PROP_DONE:
        task_row_set_done(self, g_value_get_boolean(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void task_row_finalize(GObject *object) {
    TaskRow *self = TASK_ROW(object);

    g_clear_pointer(&self->text, g_free);

    G_OBJECT_CLASS(task_row_parent_class)->finalize(object);
}

static void task_row_class_init(TaskRowClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    object_class->get_property = task_row_get_property;
    object_class->set_property = task_row_set_property;
    object_class->finalize = task_row_finalize;

    properties[PROP_TEXT] = g_param_spec_string(
        "text",
        "Text",
        "The task text",
        "",
        G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY
    );
    properties[PROP_DONE] = g_param_spec_boolean(
        "done",
        "Done",
        "Whether the task is complete",
        FALSE,
        G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY
    );
    g_object_class_install_properties(object_class, N_PROPERTIES, properties);

    signals[DELETE_REQUESTED] = g_signal_new(
        "delete-requested",
        G_TYPE_FROM_CLASS(klass),
        G_SIGNAL_RUN_LAST,
        0,
        NULL,
        NULL,
        NULL,
        G_TYPE_NONE,
        0
    );

    gtk_widget_class_set_template_from_resource(
        widget_class,
        "/com/camelofdoom/gtk4-playground/resources/task-row.ui"
    );
    gtk_widget_class_bind_template_child(widget_class, TaskRow, text_label);
    gtk_widget_class_bind_template_child(widget_class, TaskRow, done_button);
    gtk_widget_class_bind_template_child(widget_class, TaskRow, done_image);
    gtk_widget_class_bind_template_callback(
        widget_class,
        task_row_on_done_toggled
    );
    gtk_widget_class_bind_template_callback(
        widget_class,
        task_row_on_delete_clicked
    );
}

static void task_row_init(TaskRow *self) {
    gtk_widget_init_template(GTK_WIDGET(self));

    self->text = g_strdup("");
    task_row_update_appearance(self);
}

TaskRow *task_row_new(const gchar *text) {
    return g_object_new(TASK_TYPE_ROW, "text", text, NULL);
}

const gchar *task_row_get_text(TaskRow *self) {
    g_return_val_if_fail(TASK_IS_ROW(self), NULL);

    return self->text;
}

void task_row_set_text(
    TaskRow *self,
    const gchar *text
) {
    g_return_if_fail(TASK_IS_ROW(self));

    if (g_strcmp0(self->text, text) == 0) {
        return;
    }

    g_free(self->text);
    self->text = g_strdup(text != NULL ? text : "");
    task_row_update_appearance(self);
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_TEXT]);
}

gboolean task_row_get_done(TaskRow *self) {
    g_return_val_if_fail(TASK_IS_ROW(self), FALSE);

    return self->done;
}

void task_row_set_done(
    TaskRow *self,
    gboolean done
) {
    g_return_if_fail(TASK_IS_ROW(self));

    done = !!done;

    if (self->done == done) {
        return;
    }

    self->done = done;

    if (gtk_toggle_button_get_active(self->done_button) != done) {
        gtk_toggle_button_set_active(self->done_button, done);
    }

    task_row_update_appearance(self);
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_DONE]);
}
