#include "task_row.h"
#include "gtk/gtk.h"

enum {
    DELETE_REQUESTED,
    N_SIGNALS,
};

struct _TaskRow {
    GtkListBoxRow parent_instance;

    GtkLabel *text_label;
    GtkToggleButton *done_button;
    GtkImage *done_image;

    Task *task;
};

G_DEFINE_TYPE(
    TaskRow,
    task_row,
    GTK_TYPE_LIST_BOX_ROW
)

static guint signals[N_SIGNALS];

static void task_row_update_appearance(TaskRow *self) {
    PangoAttrList *attributes = NULL;

    if (!self->task)
        return;
    gtk_toggle_button_set_active(self->done_button, task_get_done(self->task));

    gtk_label_set_text(self->text_label, task_get_text(self->task));

    if (task_get_done(self->task)) {
        attributes = pango_attr_list_new();
        pango_attr_list_insert(attributes, pango_attr_strikethrough_new(TRUE));
        gtk_widget_add_css_class(GTK_WIDGET(self), "complete");
    } else {
        gtk_widget_remove_css_class(GTK_WIDGET(self), "complete");
    }

    gtk_label_set_attributes(self->text_label, attributes);

    if (attributes != NULL) {
        pango_attr_list_unref(attributes);
    }

    gtk_image_set_from_icon_name(
        self->done_image,
        task_get_done(self->task) ? "edit-undo-symbolic"
                                  : "object-select-symbolic"
    );
    gtk_widget_set_tooltip_text(
        GTK_WIDGET(self->done_button),
        task_get_done(self->task) ? "Mark task as not done"
                                  : "Mark task as done"
    );
}

static void task_row_on_done_toggled(
    GtkToggleButton *button,
    TaskRow *self
) {
    if (self->task)
        task_set_done(self->task, gtk_toggle_button_get_active(button));
}

static void task_row_on_delete_clicked(
    GtkButton *button,
    TaskRow *self
) {
    (void)button;

    g_signal_emit(self, signals[DELETE_REQUESTED], 0);
}

static void task_row_dispose(GObject *object) {
    TaskRow *self = TASK_ROW(object);
    if (self->task)
        g_signal_handlers_disconnect_by_data(self->task, self);
    g_clear_object(&self->task);
    gtk_widget_dispose_template(GTK_WIDGET(self), TASK_TYPE_ROW);
    G_OBJECT_CLASS(task_row_parent_class)->dispose(object);
}

static void on_task_changed(
    Task *task,
    GParamSpec *pspec,
    TaskRow *self
) {
    (void)task;
    (void)pspec;
    task_row_update_appearance(self);
}

static void task_row_class_init(TaskRowClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    object_class->dispose = task_row_dispose;

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
}

TaskRow *task_row_new(Task *task) {
    g_return_val_if_fail(TASK_IS_ITEM(task), NULL);
    TaskRow *self = g_object_new(TASK_TYPE_ROW, NULL);
    self->task = g_object_ref(task);
    g_signal_connect(task, "notify", G_CALLBACK(on_task_changed), self);
    task_row_update_appearance(self);
    return self;
}

Task *task_row_get_task(TaskRow *self) {
    g_return_val_if_fail(TASK_IS_ROW(self), NULL);
    return self->task;
}
