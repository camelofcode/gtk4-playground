#include "json_helpers.h"

JsonNode *json_helpers_from_string(const gchar *contents, GError **error) {
    g_autoptr(JsonParser) parser = json_parser_new();
    if (!json_parser_load_from_data(parser, contents, -1, error)) {
        return NULL;
    }
    JsonNode *root = json_parser_steal_root(parser);
    if (!root) {
        g_set_error_literal(
            error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA, "Expected a JSON value."
        );
    }
    return root;
}

gboolean json_helpers_member_has_type(
    JsonObject *object,
    const gchar *name,
    GType type
) {
    JsonNode *node = json_object_get_member(object, name);
    return node && JSON_NODE_HOLDS_VALUE(node) &&
           json_node_get_value_type(node) == type;
}

GListStore *json_helpers_deserialize_list(
    JsonNode *node,
    GType item_type,
    GError **error
) {
    if (!g_type_is_a(item_type, G_TYPE_OBJECT) ||
        G_TYPE_IS_ABSTRACT(item_type)) {
        g_set_error_literal(
            error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT,
            "Expected a concrete GObject type."
        );
        return NULL;
    }
    if (!node || !JSON_NODE_HOLDS_ARRAY(node)) {
        g_set_error_literal(
            error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA,
            "Expected a JSON array of objects."
        );
        return NULL;
    }
    g_autoptr(GListStore) items = g_list_store_new(item_type);
    JsonArray *array = json_node_get_array(node);
    for (guint i = 0; i < json_array_get_length(array); i++) {
        JsonNode *element = json_array_get_element(array, i);
        if (!JSON_NODE_HOLDS_OBJECT(element)) {
            g_set_error(
                error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA,
                "Expected a JSON object at index %u.", i
            );
            return NULL;
        }
        g_autoptr(GObject) item = json_gobject_deserialize(item_type, element);
        if (!item) {
            g_set_error(
                error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA,
                "Could not deserialize %s at index %u.", g_type_name(item_type), i
            );
            return NULL;
        }
        g_list_store_append(items, item);
    }
    return g_steal_pointer(&items);
}

void json_helpers_serialize_gobject(
    JsonBuilder *builder,
    const gchar *name,
    GObject *object
) {
    json_builder_set_member_name(builder, name);
    if (object) {
        json_builder_add_value(builder, json_gobject_serialize(object));
    } else {
        json_builder_add_null_value(builder);
    }
}

void json_helpers_serialize_gobject_list(
    JsonBuilder *builder,
    GListModel *list
) {
    json_builder_begin_array(builder);
    for (guint i = 0; i < g_list_model_get_n_items(list); i++) {
        g_autoptr(GObject) task = g_list_model_get_item(list, i);
        json_builder_add_value(builder, json_gobject_serialize(G_OBJECT(task)));
    }
    json_builder_end_array(builder);
}

void json_helpers_write_int_member(
    JsonBuilder *builder,
    const gchar *name,
    gint64 value
) {
    json_builder_set_member_name(builder, name);
    json_builder_add_int_value(builder, value);
}

void json_helpers_write_double_member(
    JsonBuilder *builder,
    const gchar *name,
    gdouble value
) {
    json_builder_set_member_name(builder, name);
    json_builder_add_double_value(builder, value);
}

void json_helpers_write_boolean_member(
    JsonBuilder *builder,
    const gchar *name,
    gboolean value
) {
    json_builder_set_member_name(builder, name);
    json_builder_add_boolean_value(builder, value);
}

void json_helpers_write_string_member(
    JsonBuilder *builder,
    const gchar *name,
    const gchar *value
) {
    json_builder_set_member_name(builder, name);
    json_builder_add_string_value(builder, value);
}

void json_helpers_write_null_member(
    JsonBuilder *builder,
    const gchar *name
) {
    json_builder_set_member_name(builder, name);
    json_builder_add_null_value(builder);
}

gchar *json_helpers_to_string(
    JsonBuilder *builder
) {
    g_autoptr(JsonNode) root = json_builder_get_root(builder);
    g_autoptr(JsonGenerator) generator = json_generator_new();
    json_generator_set_root(generator, root);
    json_generator_set_pretty(generator, TRUE);
    return json_generator_to_data(generator, NULL);
}

void json_helpers_write_nullable_int_member(
    JsonBuilder *builder,
    const gchar *name,
    const gint64 *value
) {
    if (value) {
        json_helpers_write_int_member(builder, name, *value);
    } else {
        json_helpers_write_null_member(builder, name);
    }
}

void json_helpers_write_nullable_double_member(
    JsonBuilder *builder,
    const gchar *name,
    const gdouble *value
) {
    if (value) {
        json_helpers_write_double_member(builder, name, *value);
    } else {
        json_helpers_write_null_member(builder, name);
    }
}

void json_helpers_write_nullable_boolean_member(
    JsonBuilder *builder,
    const gchar *name,
    const gboolean *value
) {
    if (value) {
        json_helpers_write_boolean_member(builder, name, *value);
    } else {
        json_helpers_write_null_member(builder, name);
    }
}
