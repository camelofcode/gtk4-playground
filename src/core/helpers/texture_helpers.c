#include "texture_helpers.h"

GObject *texture_helpers_deserialize_response(
    GBytes *response_body,
    GType texture_type,
    GError **error
) {
    g_return_val_if_fail(response_body != NULL, NULL);

    if (!g_type_is_a(texture_type, GDK_TYPE_TEXTURE)) {
        g_set_error_literal(
            error,
            G_IO_ERROR,
            G_IO_ERROR_INVALID_ARGUMENT,
            "Expected a GdkTexture response type"
        );
        return NULL;
    }

    return G_OBJECT(gdk_texture_new_from_bytes(response_body, error));
}
