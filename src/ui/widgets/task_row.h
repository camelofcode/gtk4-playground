#ifndef TASK_ROW_H
#define TASK_ROW_H

#include "src/core/task.h"
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

/* The row retains a reference; get_task returns a borrowed reference. */
TaskRow *task_row_new(Task *task);
Task *task_row_get_task(TaskRow *self);

G_END_DECLS

#endif
