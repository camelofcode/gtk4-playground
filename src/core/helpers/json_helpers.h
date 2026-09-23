#ifndef JSON_HELPERS_H
#define JSON_HELPERS_H

#include <json-glib/json-glib.h>

/* Parses NUL-terminated JSON. Returns an owned root, or NULL with error.
 * Free the result with json_node_unref() or g_autoptr(JsonNode).
 */
JsonNode *json_helpers_from_string(
    const gchar *contents,
    GError **error
);

/* Returns FALSE for missing members, non-value nodes, or a different type. */
gboolean json_helpers_member_has_type(
    JsonObject *object,
    const gchar *name,
    GType type
);

/* Deserializes an array of objects into an owned list of item_type.
 * Missing properties retain defaults; unknown properties are ignored.
 * Property conversion follows JSON-GLib (may warn and retain defaults).
 * Invalid list structure or failed construction returns NULL with error.
 */
GListStore *json_helpers_deserialize_list(
    JsonNode *node,
    GType item_type,
    GError **error
);

/* Writes a named member in an open object; NULL writes JSON null.
 * The caller retains ownership of object.
 */
void json_helpers_serialize_gobject(
    JsonBuilder *builder,
    const gchar *name,
    GObject *object
);

void json_helpers_serialize_gobject_list(
    JsonBuilder *builder,
    GListModel *list
);

void json_helpers_write_int_member(
    JsonBuilder *builder,
    const gchar *name,
    gint64 value
);

void json_helpers_write_double_member(
    JsonBuilder *builder,
    const gchar *name,
    gdouble value
);

void json_helpers_write_boolean_member(
    JsonBuilder *builder,
    const gchar *name,
    gboolean value
);

/* A NULL string writes JSON null. */
void json_helpers_write_string_member(
    JsonBuilder *builder,
    const gchar *name,
    const gchar *value
);

void json_helpers_write_null_member(
    JsonBuilder *builder,
    const gchar *name
);

/* Returns owned, NUL-terminated JSON text. */
gchar *json_helpers_to_string(JsonBuilder *builder);

/* Writes the pointed-to value, or JSON null for a NULL pointer. */
void json_helpers_write_nullable_int_member(
    JsonBuilder *builder,
    const gchar *name,
    const gint64 *value
);

/* Writes the pointed-to value, or JSON null for a NULL pointer. */
void json_helpers_write_nullable_double_member(
    JsonBuilder *builder,
    const gchar *name,
    const gdouble *value
);

/* Writes the pointed-to value, or JSON null for a NULL pointer. */
void json_helpers_write_nullable_boolean_member(
    JsonBuilder *builder,
    const gchar *name,
    const gboolean *value
);

#endif
