#ifndef TASK_ROW_H
#define TASK_ROW_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TASK_TYPE_ROW task_row_get_type()

G_DECLARE_FINAL_TYPE(
    TaskRow,
    task_row,
    TASK,
    ROW,
    GtkListBoxRow
)

TaskRow *task_row_new(const gchar *text);

const gchar *task_row_get_text(TaskRow *self);
void task_row_set_text(
    TaskRow *self,
    const gchar *text
);

gboolean task_row_get_done(TaskRow *self);
void task_row_set_done(
    TaskRow *self,
    gboolean done
);

G_END_DECLS

#endif
