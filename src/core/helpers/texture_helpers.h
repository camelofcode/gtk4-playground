#ifndef TEXTURE_HELPERS_H
#define TEXTURE_HELPERS_H

#include <gdk/gdk.h>

G_BEGIN_DECLS

/* Deserializes response_body as a GdkTexture. texture_type must be
 * GDK_TYPE_TEXTURE or one of its subtypes.
 */
GObject *texture_helpers_deserialize_response(
    GBytes *response_body,
    GType texture_type,
    GError **error
);

G_END_DECLS

#endif
