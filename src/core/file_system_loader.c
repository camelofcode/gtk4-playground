#include "file_system_loader.h"

#define FILE_ATTRIBUTES \
    G_FILE_ATTRIBUTE_STANDARD_NAME "," \
    G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME "," \
    G_FILE_ATTRIBUTE_STANDARD_TYPE "," \
    G_FILE_ATTRIBUTE_STANDARD_SIZE "," \
    G_FILE_ATTRIBUTE_TIME_MODIFIED

static FileSystemNode *load_node(
    GFile *file,
    GFileInfo *known_info,
    GCancellable *cancellable,
    GError **error
) {
    g_autoptr(GFileInfo) queried_info = NULL;
    GFileInfo *info = known_info;

    if (
        cancellable != NULL &&
        g_cancellable_set_error_if_cancelled(cancellable, error)
    ) {
        return NULL;
    }

    if (info == NULL) {
        queried_info = g_file_query_info(
            file,
            FILE_ATTRIBUTES,
            G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
            cancellable,
            error
        );
        if (queried_info == NULL) {
            return NULL;
        }
        info = queried_info;
    }

    g_autofree gchar *path = g_file_get_path(file);
    const gchar *display_name = g_file_info_get_display_name(info);
    g_autofree gchar *fallback_name = NULL;
    if (display_name == NULL) {
        fallback_name = g_file_get_basename(file);
        display_name = fallback_name;
    }

    gboolean directory =
        g_file_info_get_file_type(info) == G_FILE_TYPE_DIRECTORY;
    g_autoptr(FileSystemNode) node = file_system_node_new(
        display_name != NULL ? display_name : path,
        path,
        directory,
        g_file_info_get_size(info),
        (gint64)g_file_info_get_attribute_uint64(
            info,
            G_FILE_ATTRIBUTE_TIME_MODIFIED
        )
    );

    if (!directory) {
        return g_steal_pointer(&node);
    }

    g_autoptr(GFileEnumerator) enumerator = g_file_enumerate_children(
        file,
        FILE_ATTRIBUTES,
        G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
        cancellable,
        error
    );
    if (enumerator == NULL) {
        return NULL;
    }

    while (TRUE) {
        g_autoptr(GFileInfo) child_info = g_file_enumerator_next_file(
            enumerator,
            cancellable,
            error
        );
        if (child_info == NULL) {
            if (error != NULL && *error != NULL) {
                return NULL;
            }
            break;
        }

        g_autoptr(GFile) child_file = g_file_get_child(
            file,
            g_file_info_get_name(child_info)
        );
        g_autoptr(FileSystemNode) child = load_node(
            child_file,
            child_info,
            cancellable,
            error
        );
        if (child == NULL) {
            return NULL;
        }
        file_system_node_add_child(node, child);
    }

    file_system_node_sort_children(node);
    return g_steal_pointer(&node);
}

FileSystemNode *file_system_loader_load(
    const gchar *path,
    GCancellable *cancellable,
    GError **error
) {
    g_return_val_if_fail(path != NULL, NULL);

    g_autoptr(GFile) root = g_file_new_for_path(path);
    g_autoptr(GFileInfo) root_info = g_file_query_info(
        root,
        FILE_ATTRIBUTES,
        G_FILE_QUERY_INFO_NONE,
        cancellable,
        error
    );
    if (root_info == NULL) {
        return NULL;
    }
    return load_node(root, root_info, cancellable, error);
}

static void load_in_thread(
    GTask *task,
    gpointer source_object,
    gpointer task_data,
    GCancellable *cancellable
) {
    (void)source_object;
    const gchar *path = task_data;
    g_autoptr(GError) error = NULL;
    FileSystemNode *root = file_system_loader_load(
        path,
        cancellable,
        &error
    );

    if (root == NULL) {
        g_task_return_error(task, g_steal_pointer(&error));
    } else {
        g_task_return_pointer(task, root, g_object_unref);
    }
}

void file_system_loader_load_async(
    const gchar *path,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data
) {
    g_return_if_fail(path != NULL);

    GTask *task = g_task_new(NULL, cancellable, callback, user_data);
    g_task_set_task_data(task, g_strdup(path), g_free);
    g_task_set_return_on_cancel(task, TRUE);
    g_task_run_in_thread(task, load_in_thread);
    g_object_unref(task);
}

FileSystemNode *file_system_loader_load_finish(
    GAsyncResult *result,
    GError **error
) {
    g_return_val_if_fail(G_IS_TASK(result), NULL);
    return g_task_propagate_pointer(G_TASK(result), error);
}
