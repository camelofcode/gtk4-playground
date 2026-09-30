#include "gobject_helpers.h"

GParamSpec *gobject_helpers_string_property(const gchar *name) {
    return g_param_spec_string(name, NULL, NULL, NULL, G_PARAM_READWRITE);
}

GParamSpec *gobject_helpers_list_store_property(const gchar *name) {
    return g_param_spec_object(
        name,
        NULL,
        NULL,
        G_TYPE_LIST_STORE,
        G_PARAM_READWRITE
    );
}
