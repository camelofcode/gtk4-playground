#include "rest_helpers.h"

#include <json-glib/json-glib.h>
#include <string.h>

typedef struct {
    SoupMessage *message;
    GType response_object_type;
    GType deserializer_type;
    RestResponseDeserializer response_deserializer;
    RestResponseAsyncCallback callback;
    gpointer user_data;
} RestRequest;

static void rest_request_free(RestRequest *request) {
    g_clear_object(&request->message);
    g_free(request);
}

static void dispatch_response(
    GObject *source_object,
    GAsyncResult *result,
    gpointer user_data
) {
    (void)user_data;

    GTask *task = G_TASK(result);
    RestRequest *request = g_task_get_task_data(task);
    g_autoptr(GError) error = NULL;
    g_autoptr(GObject) response_object = g_task_propagate_pointer(task, &error);

    request->callback(
        SOUP_SESSION(source_object),
        response_object,
        error,
        request->user_data
    );
}

static void parse_response(
    GObject *source_object,
    GAsyncResult *result,
    gpointer user_data
) {
    GTask *task = G_TASK(user_data);
    RestRequest *request = g_task_get_task_data(task);
    g_autoptr(GError) error = NULL;
    g_autoptr(GBytes) response_body = soup_session_send_and_read_finish(
        SOUP_SESSION(source_object),
        result,
        &error
    );

    if (!response_body) {
        g_task_return_error(task, g_steal_pointer(&error));
        g_object_unref(task);
        return;
    }

    guint status = soup_message_get_status(request->message);
    if (!SOUP_STATUS_IS_SUCCESSFUL(status)) {
        g_task_return_new_error(
            task,
            G_IO_ERROR,
            G_IO_ERROR_FAILED,
            "REST request failed (HTTP %u)",
            status
        );
        g_object_unref(task);
        return;
    }

    GObject *response_object = NULL;
    if (request->response_deserializer) {
        response_object = request->response_deserializer(
            response_body,
            request->deserializer_type,
            &error
        );
    } else {
        gsize response_length = 0;
        gconstpointer response_data =
            g_bytes_get_data(response_body, &response_length);
        response_object = json_gobject_from_data(
            request->response_object_type,
            response_data,
            response_length,
            &error
        );
    }
    if (!response_object && !error) {
        g_set_error_literal(
            &error,
            G_IO_ERROR,
            G_IO_ERROR_INVALID_DATA,
            "Could not deserialize REST response"
        );
    }

    if (response_object) {
        g_task_return_pointer(task, response_object, g_object_unref);
    } else {
        g_task_return_error(task, g_steal_pointer(&error));
    }
    g_object_unref(task);
}

static gboolean validate_arguments(
    GTask *task,
    const gchar *url,
    const gchar *method,
    GType response_object_type,
    GType deserializer_type,
    RestResponseDeserializer response_deserializer
) {
    if (!url || !*url) {
        g_task_return_new_error(
            task,
            G_IO_ERROR,
            G_IO_ERROR_INVALID_ARGUMENT,
            "REST request URL is empty"
        );
        return FALSE;
    }
    if (!method || !*method) {
        g_task_return_new_error(
            task,
            G_IO_ERROR,
            G_IO_ERROR_INVALID_ARGUMENT,
            "REST request method is empty"
        );
        return FALSE;
    }
    if (response_deserializer &&
        !g_type_is_a(deserializer_type, G_TYPE_OBJECT)) {
        g_task_return_new_error(
            task,
            G_IO_ERROR,
            G_IO_ERROR_INVALID_ARGUMENT,
            "REST deserializer type must be a GObject type"
        );
        return FALSE;
    }
    if (!response_deserializer &&
        (!g_type_is_a(response_object_type, G_TYPE_OBJECT) ||
         G_TYPE_IS_ABSTRACT(response_object_type))) {
        g_task_return_new_error(
            task,
            G_IO_ERROR,
            G_IO_ERROR_INVALID_ARGUMENT,
            "REST response requires a concrete GObject type or deserializer"
        );
        return FALSE;
    }
    return TRUE;
}

static void rest_send_async(
    SoupSession *session,
    const gchar *url,
    const gchar *method,
    GObject *request_object,
    GType response_object_type,
    GType deserializer_type,
    RestResponseDeserializer response_deserializer,
    GCancellable *cancellable,
    RestResponseAsyncCallback callback,
    gpointer user_data
) {
    g_return_if_fail(SOUP_IS_SESSION(session));
    g_return_if_fail(!cancellable || G_IS_CANCELLABLE(cancellable));
    g_return_if_fail(callback != NULL);

    RestRequest *request = g_new0(RestRequest, 1);
    request->response_object_type = response_object_type;
    request->deserializer_type = deserializer_type;
    request->response_deserializer = response_deserializer;
    request->callback = callback;
    request->user_data = user_data;

    GTask *task = g_task_new(session, cancellable, dispatch_response, NULL);
    g_task_set_task_data(task, request, (GDestroyNotify)rest_request_free);

    if (!validate_arguments(
            task,
            url,
            method,
            response_object_type,
            deserializer_type,
            response_deserializer
        )) {
        g_object_unref(task);
        return;
    }

    request->message = soup_message_new(method, url);
    if (!request->message) {
        g_task_return_new_error(
            task,
            G_IO_ERROR,
            G_IO_ERROR_INVALID_ARGUMENT,
            "Could not parse REST request URL: %s",
            url
        );
        g_object_unref(task);
        return;
    }

    if (request_object) {
        g_autoptr(JsonNode) request_node =
            json_gobject_serialize(request_object);
        g_autofree gchar *request_json = json_to_string(request_node, FALSE);
        g_autoptr(GBytes) request_bytes =
            g_bytes_new(request_json, strlen(request_json));
        soup_message_set_request_body_from_bytes(
            request->message,
            "application/json",
            request_bytes
        );
    }

    soup_session_send_and_read_async(
        session,
        request->message,
        G_PRIORITY_DEFAULT,
        cancellable,
        parse_response,
        task
    );
}

void rest_send_json_async(
    SoupSession *session,
    const gchar *url,
    const gchar *method,
    GObject *request_object,
    GType response_object_type,
    GCancellable *cancellable,
    RestResponseAsyncCallback callback,
    gpointer user_data
) {
    rest_send_async(
        session,
        url,
        method,
        request_object,
        response_object_type,
        G_TYPE_INVALID,
        NULL,
        cancellable,
        callback,
        user_data
    );
}

void rest_get_json_async(
    SoupSession *session,
    const gchar *url,
    GType response_object_type,
    GCancellable *cancellable,
    RestResponseAsyncCallback callback,
    gpointer user_data
) {
    rest_send_async(
        session,
        url,
        "GET",
        NULL,
        response_object_type,
        G_TYPE_INVALID,
        NULL,
        cancellable,
        callback,
        user_data
    );
}

void rest_get_with_deserializer_async(
    SoupSession *session,
    const gchar *url,
    GType deserializer_type,
    RestResponseDeserializer response_deserializer,
    GCancellable *cancellable,
    RestResponseAsyncCallback callback,
    gpointer user_data
) {
    rest_send_async(
        session,
        url,
        "GET",
        NULL,
        G_TYPE_INVALID,
        deserializer_type,
        response_deserializer,
        cancellable,
        callback,
        user_data
    );
}

void rest_post_json_async(
    SoupSession *session,
    const gchar *url,
    GObject *request_object,
    GType response_object_type,
    GCancellable *cancellable,
    RestResponseAsyncCallback callback,
    gpointer user_data
) {
    rest_send_json_async(
        session,
        url,
        "POST",
        request_object,
        response_object_type,
        cancellable,
        callback,
        user_data
    );
}
