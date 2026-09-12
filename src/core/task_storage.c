#include "task_storage.h"
#include <errno.h>
#include <glib/gstdio.h>
#include <json-glib/json-glib.h>

static gboolean member_has_type(
    JsonObject *object,
    const gchar *name,
    GType type
) {
    JsonNode *node = json_object_get_member(object, name);
    return node && JSON_NODE_HOLDS_VALUE(node) &&
           json_node_get_value_type(node) == type;
}

gchar *task_storage_default_path(void) {
    return g_build_filename(
        g_get_user_data_dir(),
        "gtk4-playground",
        "tasks.json",
        NULL
    );
}

GListStore *task_storage_load(
    const gchar *path,
    GError **error
) {
    g_autoptr(GFile) file = g_file_new_for_path(path);
    g_autofree gchar *contents = NULL;
    gsize length;
    g_autoptr(GError) local_error = NULL;
    g_autoptr(GListStore) tasks = g_list_store_new(TASK_TYPE_ITEM);
    if (!g_file_load_contents(
            file,
            NULL,
            &contents,
            &length,
            NULL,
            &local_error
        )) {
        if (g_error_matches(local_error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND))
            return g_steal_pointer(&tasks);
        g_propagate_error(error, g_steal_pointer(&local_error));
        return NULL;
    }

    g_autoptr(JsonParser) parser = json_parser_new();
    if (!json_parser_load_from_data(parser, contents, length, error))
        return NULL;
    JsonNode *root = json_parser_get_root(parser);
    if (!root || !JSON_NODE_HOLDS_OBJECT(root))
        goto invalid;
    JsonObject *object = json_node_get_object(root);
    if (!member_has_type(object, "version", G_TYPE_INT64) ||
        json_object_get_int_member(object, "version") != 1)
        goto invalid;
    JsonNode *node = json_object_get_member(object, "tasks");
    if (!node || !JSON_NODE_HOLDS_ARRAY(node))
        goto invalid;
    JsonArray *array = json_node_get_array(node);
    for (guint i = 0; i < json_array_get_length(array); i++) {
        node = json_array_get_element(array, i);
        if (!JSON_NODE_HOLDS_OBJECT(node))
            goto invalid;
        object = json_node_get_object(node);
        if (!member_has_type(object, "text", G_TYPE_STRING) ||
            !member_has_type(object, "done", G_TYPE_BOOLEAN))
            goto invalid;
        g_autoptr(Task) task =
            task_new(json_object_get_string_member(object, "text"));
        task_set_done(task, json_object_get_boolean_member(object, "done"));
        g_list_store_append(tasks, task);
    }
    return g_steal_pointer(&tasks);

invalid:
    g_set_error_literal(
        error,
        G_IO_ERROR,
        G_IO_ERROR_INVALID_DATA,
        "Expected a version 1 task file with text and done fields."
    );
    return NULL;
}

gboolean task_storage_save(
    GListModel *tasks,
    const gchar *path,
    GError **error
) {
    g_autoptr(JsonBuilder) builder = json_builder_new();
    json_builder_begin_object(builder);
    json_builder_set_member_name(builder, "version");
    json_builder_add_int_value(builder, 1);
    json_builder_set_member_name(builder, "tasks");
    json_builder_begin_array(builder);
    for (guint i = 0; i < g_list_model_get_n_items(tasks); i++) {
        g_autoptr(Task) task = g_list_model_get_item(tasks, i);
        json_builder_begin_object(builder);
        json_builder_set_member_name(builder, "text");
        json_builder_add_string_value(builder, task_get_text(task));
        json_builder_set_member_name(builder, "done");
        json_builder_add_boolean_value(builder, task_get_done(task));
        json_builder_end_object(builder);
    }
    json_builder_end_array(builder);
    json_builder_end_object(builder);
    g_autoptr(JsonNode) root = json_builder_get_root(builder);
    g_autoptr(JsonGenerator) generator = json_generator_new();
    json_generator_set_root(generator, root);
    json_generator_set_pretty(generator, TRUE);
    gsize length;
    g_autofree gchar *contents = json_generator_to_data(generator, &length);
    g_autofree gchar *directory = g_path_get_dirname(path);
    if (g_mkdir_with_parents(directory, 0700) < 0) {
        g_set_error(
            error,
            G_FILE_ERROR,
            g_file_error_from_errno(errno),
            "Could not create task directory: %s",
            g_strerror(errno)
        );
        return FALSE;
    }
    g_autoptr(GFile) file = g_file_new_for_path(path);
    return g_file_replace_contents(
        file,
        contents,
        length,
        NULL,
        FALSE,
        G_FILE_CREATE_PRIVATE,
        NULL,
        NULL,
        error
    );
}
