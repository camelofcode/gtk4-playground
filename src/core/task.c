#include "task.h"

enum {
    PROP_0,
    PROP_TEXT,
    PROP_DONE,
    N_PROPERTIES,
};

struct _Task {
    GObject parent_instance;
    gchar *text;
    gboolean done;
};

G_DEFINE_TYPE(
    Task,
    task,
    G_TYPE_OBJECT
)
static GParamSpec *properties[N_PROPERTIES];

static void task_get_property(
    GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec
) {
    Task *self = TASK_ITEM(object);

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

static void task_set_property(
    GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec
) {
    Task *self = TASK_ITEM(object);

    switch (property_id) {
    case PROP_TEXT:
        task_set_text(self, g_value_get_string(value));
        break;
    case PROP_DONE:
        task_set_done(self, g_value_get_boolean(value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void task_finalize(GObject *object) {
    Task *self = TASK_ITEM(object);

    g_clear_pointer(&self->text, g_free);

    G_OBJECT_CLASS(task_parent_class)->finalize(object);
}

static void task_class_init(TaskClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->get_property = task_get_property;
    object_class->set_property = task_set_property;
    object_class->finalize = task_finalize;
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
}

static void task_init(Task *self) { self->text = g_strdup(""); }

Task *task_new(const gchar *text) {
    return g_object_new(TASK_TYPE_ITEM, "text", text, NULL);
}

const gchar *task_get_text(Task *self) {
    g_return_val_if_fail(TASK_IS_ITEM(self), NULL);

    return self->text;
}

void task_set_text(
    Task *self,
    const gchar *text
) {
    g_return_if_fail(TASK_IS_ITEM(self));

    if (g_strcmp0(self->text, text) == 0) {
        return;
    }

    g_free(self->text);
    self->text = g_strdup(text != NULL ? text : "");
    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_TEXT]);
}

gboolean task_get_done(Task *self) {
    g_return_val_if_fail(TASK_IS_ITEM(self), FALSE);

    return self->done;
}

void task_set_done(
    Task *self,
    gboolean done
) {
    g_return_if_fail(TASK_IS_ITEM(self));

    done = !!done;

    if (self->done == done) {
        return;
    }

    self->done = done;

    g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_DONE]);
}
