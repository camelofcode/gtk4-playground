#ifndef TASK_H
#define TASK_H

#include <glib-object.h>

G_BEGIN_DECLS

#define TASK_TYPE_ITEM task_get_type()

G_DECLARE_FINAL_TYPE(
    Task,
    task,
    TASK,
    ITEM,
    GObject
)

Task *task_new(const gchar *text);

const gchar *task_get_text(Task *self);
void task_set_text(
    Task *self,
    const gchar *text
);

gboolean task_get_done(Task *self);
void task_set_done(
    Task *self,
    gboolean done
);

G_END_DECLS

#endif
