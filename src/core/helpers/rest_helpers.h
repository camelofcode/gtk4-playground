#ifndef REST_HELPERS_H
#define REST_HELPERS_H

#include <gio/gio.h>
#include <libsoup/soup.h>

G_BEGIN_DECLS

/**
 * RestResponseAsyncCallback:
 * @session: (transfer none): the #SoupSession that performed the request
 * @response_object: (nullable) (transfer none): the deserialized response
 *   object, or %NULL
 * @error: (nullable) (transfer none): the request or deserialization error, or
 *   %NULL
 * @user_data: (nullable): caller-owned data supplied when starting the request
 *
 * Called when a REST request completes. On success, @error is %NULL and
 * @response_object contains the deserialized object. Requests that do not
 * expect a response object complete successfully with both values set to
 * %NULL. On failure, @response_object is %NULL and @error is set.
 *
 * The arguments are borrowed and valid only for the duration of the callback.
 * The callback is always invoked asynchronously in the thread-default main
 * context of the initiating call. @user_data must remain valid until then.
 */
typedef void (*RestResponseAsyncCallback)(
    SoupSession *session,
    GObject *response_object,
    const GError *error,
    gpointer user_data
);

/**
 * RestResponseDeserializer:
 * @response_body: (transfer none): the response body to deserialize
 * @deserializer_type: a #GType supplied by the request function
 * @error: (out) (optional): return location for a #GError, or %NULL
 *
 * Deserializes a successful REST response. @deserializer_type identifies the
 * #GObject type needed by the implementation and does not have to be the type
 * of the returned object. For example, it may identify the item type contained
 * by a returned collection object.
 *
 * Returns: (transfer full) (nullable): a newly allocated #GObject, or %NULL
 *   with @error set
 */
typedef GObject *(*RestResponseDeserializer)(
    GBytes *response_body,
    GType deserializer_type,
    GError **error
);

/**
 * rest_get_async:
 * @session: (transfer none): the #SoupSession used to send the request
 * @url: the request URL
 * @response_object_type: the concrete #GType used to deserialize the JSON
 *   response
 * @cancellable: (nullable) (transfer none): optional #GCancellable
 * @callback: (scope async) (closure user_data): function called when the
 *   request completes
 * @user_data: (nullable): caller-owned data passed to @callback
 *
 * Sends a GET request without a request body. A successful response body is
 * parsed as JSON and deserialized as @response_object_type. Transport,
 * cancellation, HTTP-status, and deserialization failures are reported to
 * @callback.
 */
void rest_get_async(
    SoupSession *session,
    const gchar *url,
    GType response_object_type,
    GCancellable *cancellable,
    RestResponseAsyncCallback callback,
    gpointer user_data
);

/**
 * rest_get_no_response_async:
 * @session: (transfer none): the #SoupSession used to send the request
 * @url: the request URL
 * @cancellable: (nullable) (transfer none): optional #GCancellable
 * @callback: (scope async) (closure user_data): function called when the
 *   request completes
 * @user_data: (nullable): caller-owned data passed to @callback
 *
 * Sends a GET request without a request body or expected response object. On a
 * successful HTTP status, the response body is ignored and @callback receives
 * a %NULL response object and %NULL error. Request failures are reported to
 * @callback.
 */
void rest_get_no_response_async(
    SoupSession *session,
    const gchar *url,
    GCancellable *cancellable,
    RestResponseAsyncCallback callback,
    gpointer user_data
);

/**
 * rest_get_with_response_deserializer_async:
 * @session: (transfer none): the #SoupSession used to send the request
 * @url: the request URL
 * @deserializer_type: the #GType passed to @response_deserializer
 * @response_deserializer: (scope async): function used to deserialize the
 *   response body
 * @cancellable: (nullable) (transfer none): optional #GCancellable
 * @callback: (scope async) (closure user_data): function called when the
 *   request completes
 * @user_data: (nullable): caller-owned data passed to @callback
 *
 * Sends a GET request without a request body. On a successful HTTP status,
 * @response_deserializer receives the response body and @deserializer_type.
 * Request and deserialization failures are reported to @callback.
 */
void rest_get_with_response_deserializer_async(
    SoupSession *session,
    const gchar *url,
    GType deserializer_type,
    RestResponseDeserializer response_deserializer,
    GCancellable *cancellable,
    RestResponseAsyncCallback callback,
    gpointer user_data
);

/**
 * rest_post_async:
 * @session: (transfer none): the #SoupSession used to send the request
 * @url: the request URL
 * @request_object: (nullable) (transfer none): object serialized as the JSON
 *   request body, or %NULL for no request body
 * @response_object_type: the concrete #GType used to deserialize the JSON
 *   response
 * @cancellable: (nullable) (transfer none): optional #GCancellable
 * @callback: (scope async) (closure user_data): function called when the
 *   request completes
 * @user_data: (nullable): caller-owned data passed to @callback
 *
 * Sends a POST request. @request_object is serialized before this function
 * returns and is not retained. A successful response body is parsed as JSON
 * and deserialized as @response_object_type. Request and deserialization
 * failures are reported to @callback.
 */
void rest_post_async(
    SoupSession *session,
    const gchar *url,
    GObject *request_object,
    GType response_object_type,
    GCancellable *cancellable,
    RestResponseAsyncCallback callback,
    gpointer user_data
);

/**
 * rest_post_no_response_async:
 * @session: (transfer none): the #SoupSession used to send the request
 * @url: the request URL
 * @request_object: (nullable) (transfer none): object serialized as the JSON
 *   request body, or %NULL for no request body
 * @cancellable: (nullable) (transfer none): optional #GCancellable
 * @callback: (scope async) (closure user_data): function called when the
 *   request completes
 * @user_data: (nullable): caller-owned data passed to @callback
 *
 * Sends a POST request. @request_object is serialized before this function
 * returns and is not retained. On a successful HTTP status, the response body
 * is ignored and @callback receives a %NULL response object and %NULL error.
 * Request failures are reported to @callback.
 */
void rest_post_no_response_async(
    SoupSession *session,
    const gchar *url,
    GObject *request_object,
    GCancellable *cancellable,
    RestResponseAsyncCallback callback,
    gpointer user_data
);

/**
 * rest_post_with_response_deserializer_async:
 * @session: (transfer none): the #SoupSession used to send the request
 * @url: the request URL
 * @request_object: (nullable) (transfer none): object serialized as the JSON
 *   request body, or %NULL for no request body
 * @deserializer_type: the #GType passed to @response_deserializer
 * @response_deserializer: (scope async): function used to deserialize the
 *   response body
 * @cancellable: (nullable) (transfer none): optional #GCancellable
 * @callback: (scope async) (closure user_data): function called when the
 *   request completes
 * @user_data: (nullable): caller-owned data passed to @callback
 *
 * Sends a POST request. @request_object is serialized before this function
 * returns and is not retained. On a successful HTTP status,
 * @response_deserializer receives the response body and @deserializer_type.
 * Request and deserialization failures are reported to @callback.
 */
void rest_post_with_response_deserializer_async(
    SoupSession *session,
    const gchar *url,
    GObject *request_object,
    GType deserializer_type,
    RestResponseDeserializer response_deserializer,
    GCancellable *cancellable,
    RestResponseAsyncCallback callback,
    gpointer user_data
);

/**
 * rest_put_async:
 * @session: (transfer none): the #SoupSession used to send the request
 * @url: the request URL
 * @request_object: (nullable) (transfer none): object serialized as the JSON
 *   request body, or %NULL for no request body
 * @response_object_type: the concrete #GType used to deserialize the JSON
 *   response
 * @cancellable: (nullable) (transfer none): optional #GCancellable
 * @callback: (scope async) (closure user_data): function called when the
 *   request completes
 * @user_data: (nullable): caller-owned data passed to @callback
 *
 * Sends a PUT request. @request_object is serialized before this function
 * returns and is not retained. A successful response body is parsed as JSON
 * and deserialized as @response_object_type. Request and deserialization
 * failures are reported to @callback.
 */
void rest_put_async(
    SoupSession *session,
    const gchar *url,
    GObject *request_object,
    GType response_object_type,
    GCancellable *cancellable,
    RestResponseAsyncCallback callback,
    gpointer user_data
);

/**
 * rest_put_no_response_async:
 * @session: (transfer none): the #SoupSession used to send the request
 * @url: the request URL
 * @request_object: (nullable) (transfer none): object serialized as the JSON
 *   request body, or %NULL for no request body
 * @cancellable: (nullable) (transfer none): optional #GCancellable
 * @callback: (scope async) (closure user_data): function called when the
 *   request completes
 * @user_data: (nullable): caller-owned data passed to @callback
 *
 * Sends a PUT request. @request_object is serialized before this function
 * returns and is not retained. On a successful HTTP status, the response body
 * is ignored and @callback receives a %NULL response object and %NULL error.
 * Request failures are reported to @callback.
 */
void rest_put_no_response_async(
    SoupSession *session,
    const gchar *url,
    GObject *request_object,
    GCancellable *cancellable,
    RestResponseAsyncCallback callback,
    gpointer user_data
);

/**
 * rest_put_with_response_deserializer_async:
 * @session: (transfer none): the #SoupSession used to send the request
 * @url: the request URL
 * @request_object: (nullable) (transfer none): object serialized as the JSON
 *   request body, or %NULL for no request body
 * @deserializer_type: the #GType passed to @response_deserializer
 * @response_deserializer: (scope async): function used to deserialize the
 *   response body
 * @cancellable: (nullable) (transfer none): optional #GCancellable
 * @callback: (scope async) (closure user_data): function called when the
 *   request completes
 * @user_data: (nullable): caller-owned data passed to @callback
 *
 * Sends a PUT request. @request_object is serialized before this function
 * returns and is not retained. On a successful HTTP status,
 * @response_deserializer receives the response body and @deserializer_type.
 * Request and deserialization failures are reported to @callback.
 */
void rest_put_with_response_deserializer_async(
    SoupSession *session,
    const gchar *url,
    GObject *request_object,
    GType deserializer_type,
    RestResponseDeserializer response_deserializer,
    GCancellable *cancellable,
    RestResponseAsyncCallback callback,
    gpointer user_data
);

/**
 * rest_patch_async:
 * @session: (transfer none): the #SoupSession used to send the request
 * @url: the request URL
 * @request_object: (nullable) (transfer none): object serialized as the JSON
 *   request body, or %NULL for no request body
 * @response_object_type: the concrete #GType used to deserialize the JSON
 *   response
 * @cancellable: (nullable) (transfer none): optional #GCancellable
 * @callback: (scope async) (closure user_data): function called when the
 *   request completes
 * @user_data: (nullable): caller-owned data passed to @callback
 *
 * Sends a PATCH request. @request_object is serialized before this function
 * returns and is not retained. A successful response body is parsed as JSON
 * and deserialized as @response_object_type. Request and deserialization
 * failures are reported to @callback.
 */
void rest_patch_async(
    SoupSession *session,
    const gchar *url,
    GObject *request_object,
    GType response_object_type,
    GCancellable *cancellable,
    RestResponseAsyncCallback callback,
    gpointer user_data
);

/**
 * rest_patch_no_response_async:
 * @session: (transfer none): the #SoupSession used to send the request
 * @url: the request URL
 * @request_object: (nullable) (transfer none): object serialized as the JSON
 *   request body, or %NULL for no request body
 * @cancellable: (nullable) (transfer none): optional #GCancellable
 * @callback: (scope async) (closure user_data): function called when the
 *   request completes
 * @user_data: (nullable): caller-owned data passed to @callback
 *
 * Sends a PATCH request. @request_object is serialized before this function
 * returns and is not retained. On a successful HTTP status, the response body
 * is ignored and @callback receives a %NULL response object and %NULL error.
 * Request failures are reported to @callback.
 */
void rest_patch_no_response_async(
    SoupSession *session,
    const gchar *url,
    GObject *request_object,
    GCancellable *cancellable,
    RestResponseAsyncCallback callback,
    gpointer user_data
);

/**
 * rest_patch_with_response_deserializer_async:
 * @session: (transfer none): the #SoupSession used to send the request
 * @url: the request URL
 * @request_object: (nullable) (transfer none): object serialized as the JSON
 *   request body, or %NULL for no request body
 * @deserializer_type: the #GType passed to @response_deserializer
 * @response_deserializer: (scope async): function used to deserialize the
 *   response body
 * @cancellable: (nullable) (transfer none): optional #GCancellable
 * @callback: (scope async) (closure user_data): function called when the
 *   request completes
 * @user_data: (nullable): caller-owned data passed to @callback
 *
 * Sends a PATCH request. @request_object is serialized before this function
 * returns and is not retained. On a successful HTTP status,
 * @response_deserializer receives the response body and @deserializer_type.
 * Request and deserialization failures are reported to @callback.
 */
void rest_patch_with_response_deserializer_async(
    SoupSession *session,
    const gchar *url,
    GObject *request_object,
    GType deserializer_type,
    RestResponseDeserializer response_deserializer,
    GCancellable *cancellable,
    RestResponseAsyncCallback callback,
    gpointer user_data
);

/**
 * rest_delete_async:
 * @session: (transfer none): the #SoupSession used to send the request
 * @url: the request URL
 * @response_object_type: the concrete #GType used to deserialize the JSON
 *   response
 * @cancellable: (nullable) (transfer none): optional #GCancellable
 * @callback: (scope async) (closure user_data): function called when the
 *   request completes
 * @user_data: (nullable): caller-owned data passed to @callback
 *
 * Sends a DELETE request without a request body. A successful response body is
 * parsed as JSON and deserialized as @response_object_type. Request and
 * deserialization failures are reported to @callback.
 */
void rest_delete_async(
    SoupSession *session,
    const gchar *url,
    GType response_object_type,
    GCancellable *cancellable,
    RestResponseAsyncCallback callback,
    gpointer user_data
);

/**
 * rest_delete_no_response_async:
 * @session: (transfer none): the #SoupSession used to send the request
 * @url: the request URL
 * @cancellable: (nullable) (transfer none): optional #GCancellable
 * @callback: (scope async) (closure user_data): function called when the
 *   request completes
 * @user_data: (nullable): caller-owned data passed to @callback
 *
 * Sends a DELETE request without a request body or expected response object.
 * On a successful HTTP status, the response body is ignored and @callback
 * receives a %NULL response object and %NULL error. Request failures are
 * reported to @callback.
 */
void rest_delete_no_response_async(
    SoupSession *session,
    const gchar *url,
    GCancellable *cancellable,
    RestResponseAsyncCallback callback,
    gpointer user_data
);

/**
 * rest_delete_with_response_deserializer_async:
 * @session: (transfer none): the #SoupSession used to send the request
 * @url: the request URL
 * @deserializer_type: the #GType passed to @response_deserializer
 * @response_deserializer: (scope async): function used to deserialize the
 *   response body
 * @cancellable: (nullable) (transfer none): optional #GCancellable
 * @callback: (scope async) (closure user_data): function called when the
 *   request completes
 * @user_data: (nullable): caller-owned data passed to @callback
 *
 * Sends a DELETE request without a request body. On a successful HTTP status,
 * @response_deserializer receives the response body and @deserializer_type.
 * Request and deserialization failures are reported to @callback.
 */
void rest_delete_with_response_deserializer_async(
    SoupSession *session,
    const gchar *url,
    GType deserializer_type,
    RestResponseDeserializer response_deserializer,
    GCancellable *cancellable,
    RestResponseAsyncCallback callback,
    gpointer user_data
);

G_END_DECLS

#endif
