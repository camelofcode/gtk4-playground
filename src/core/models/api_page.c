#include "api_page.h"

#include "src/core/helpers/json_helpers.h"

struct _ApiPage {
    GObject parent_instance;
    guint current_page;
    guint total_pages;
    GListModel *items;
};

G_DEFINE_TYPE(
    ApiPage,
    api_page,
    G_TYPE_OBJECT
)

static void api_page_finalize(GObject *object) {
    ApiPage *self = API_PAGE(object);
    g_clear_object(&self->items);
    G_OBJECT_CLASS(api_page_parent_class)->finalize(object);
}

static void api_page_class_init(ApiPageClass *klass) {
    G_OBJECT_CLASS(klass)->finalize = api_page_finalize;
}

static void api_page_init(ApiPage *self) { (void)self; }

ApiPage *api_page_new(
    guint current_page,
    guint total_pages,
    GListModel *items
) {
    g_return_val_if_fail(items == NULL || G_IS_LIST_MODEL(items), NULL);

    ApiPage *self = g_object_new(API_TYPE_PAGE, NULL);
    self->current_page = current_page;
    self->total_pages = total_pages;
    self->items = items ? g_object_ref(items) : NULL;
    return self;
}

static gboolean api_page_read_integer(
    JsonObject *object,
    const gchar *member,
    guint *value,
    GError **error
) {
    if (!json_helpers_member_has_type(object, member, G_TYPE_INT64)) {
        g_set_error(
            error,
            G_IO_ERROR,
            G_IO_ERROR_INVALID_DATA,
            "API page is missing integer member '%s'",
            member
        );
        return FALSE;
    }

    JsonNode *node = json_object_get_member(object, member);
    gint64 integer = json_node_get_int(node);
    if (integer < 1 || integer > G_MAXUINT) {
        g_set_error(
            error,
            G_IO_ERROR,
            G_IO_ERROR_INVALID_DATA,
            "API page member '%s' is outside the supported range",
            member
        );
        return FALSE;
    }

    *value = (guint)integer;
    return TRUE;
}

ApiPage *api_page_from_json(
    const gchar *json,
    gssize length,
    GType item_type,
    GError **error
) {
    g_return_val_if_fail(json != NULL, NULL);
    g_return_val_if_fail(g_type_is_a(item_type, G_TYPE_OBJECT), NULL);

    g_autoptr(JsonNode) root = json_helpers_from_data(json, length, error);
    if (!root) {
        return NULL;
    }

    if (!JSON_NODE_HOLDS_OBJECT(root)) {
        g_set_error_literal(
            error,
            G_IO_ERROR,
            G_IO_ERROR_INVALID_DATA,
            "API page response is not a JSON object"
        );
        return NULL;
    }

    JsonObject *object = json_node_get_object(root);
    guint current_page = 0;
    guint total_pages = 0;
    if (!api_page_read_integer(object, "currentPage", &current_page, error) ||
        !api_page_read_integer(object, "totalPages", &total_pages, error)) {
        return NULL;
    }

    JsonNode *items_node = json_object_get_member(object, "items");
    g_autoptr(GListStore) items =
        json_helpers_deserialize_list(items_node, item_type, error);
    if (!items) {
        return NULL;
    }

    return api_page_new(current_page, total_pages, G_LIST_MODEL(items));
}

GObject *api_page_deserialize_response(
    GBytes *response_body,
    GType item_type,
    GError **error
) {
    g_return_val_if_fail(response_body != NULL, NULL);

    gsize response_length = 0;
    gconstpointer response_data =
        g_bytes_get_data(response_body, &response_length);
    return G_OBJECT(
        api_page_from_json(response_data, response_length, item_type, error)
    );
}

guint api_page_get_current_page(ApiPage *self) {
    g_return_val_if_fail(API_IS_PAGE(self), 0);
    return self->current_page;
}

guint api_page_get_total_pages(ApiPage *self) {
    g_return_val_if_fail(API_IS_PAGE(self), 0);
    return self->total_pages;
}

GListModel *api_page_get_items(ApiPage *self) {
    g_return_val_if_fail(API_IS_PAGE(self), NULL);
    return self->items;
}
