#include "task_storage.h"
#include "helpers/file_helpers.h"
#include "helpers/json_helpers.h"
#include "task.h"

static gchar *task_storage_default_path(void) {
    return g_build_filename(
        g_get_user_data_dir(),
        "gtk4-playground",
        "tasks.json",
        NULL
    );
}

GListStore *task_storage_load(
    GError **error
) {
    g_autofree gchar *path = task_storage_default_path();
    g_autoptr(GError) local_error = NULL;
    g_autofree gchar *contents = file_helpers_read_all_text(path, &local_error);
    if (!contents) {
        if (g_error_matches(local_error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND)) {
            return g_list_store_new(TASK_TYPE_ITEM);
        }
        g_propagate_error(error, g_steal_pointer(&local_error));
        return NULL;
    }

    g_autoptr(JsonNode) root = json_helpers_from_string(contents, error);
    if (!root) {
        return NULL;
    }
    return json_helpers_deserialize_list(root, TASK_TYPE_ITEM, error);
}

gboolean task_storage_save(
    GListModel *tasks,
    GError **error
) {
    g_autofree gchar *path = task_storage_default_path();
    g_autoptr(JsonBuilder) builder = json_builder_new();
    json_helpers_serialize_gobject_list(builder, tasks);

    g_autofree gchar *str = json_helpers_to_string(builder);
    return file_helpers_write_all_text(path, str, error);
}
