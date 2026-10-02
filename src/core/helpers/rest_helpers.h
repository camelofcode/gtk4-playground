#ifndef REST_HELPERS_H
#define REST_HELPERS_H

#include <gio/gio.h>
#include <libsoup/soup.h>

G_BEGIN_DECLS

/* response_object and error are borrowed and valid only during the callback.
 * Exactly one of them is non-NULL. The callback is always invoked
 * asynchronously in the thread-default main context of the initiating call.
 * user_data remains caller-owned and must stay valid until the callback.
 */
typedef void (*RestResponseAsyncCallback)(
    SoupSession *session,
    GObject *response_object,
    const GError *error,
    gpointer user_data
);

/* response_body is borrowed for the duration of the call. Returns a newly
 * allocated GObject, or NULL with error set.
 */
typedef GObject *(*RestResponseDeserializer)(
    GBytes *response_body,
    GType deserializer_type,
    GError **error
);

/* A non-NULL request_object is serialized before this function returns and is
 * not retained by the asynchronous operation. NULL sends no request body.
 */
void rest_send_json_async(
    SoupSession *session,
    const gchar *url,
    const gchar *method,
    GObject *request_object,
    GType response_object_type,
    GCancellable *cancellable,
    RestResponseAsyncCallback callback,
    gpointer user_data
);

void rest_get_json_async(
    SoupSession *session,
    const gchar *url,
    GType response_object_type,
    GCancellable *cancellable,
    RestResponseAsyncCallback callback,
    gpointer user_data
);

void rest_get_with_deserializer_async(
    SoupSession *session,
    const gchar *url,
    GType deserializer_type,
    RestResponseDeserializer response_deserializer,
    GCancellable *cancellable,
    RestResponseAsyncCallback callback,
    gpointer user_data
);

void rest_post_json_async(
    SoupSession *session,
    const gchar *url,
    GObject *request_object,
    GType response_object_type,
    GCancellable *cancellable,
    RestResponseAsyncCallback callback,
    gpointer user_data
);

G_END_DECLS

#endif
