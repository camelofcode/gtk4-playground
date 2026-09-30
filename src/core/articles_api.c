#include "articles_api.h"

#include "models/article_summary.h"
#include <json-glib/json-glib.h>
#include <libsoup/soup.h>
#include <string.h>

struct _ArticlesApi {
    GObject parent_instance;
    SoupSession *session;
    gchar *base_url;
};
G_DEFINE_TYPE(
    ArticlesApi,
    articles_api,
    G_TYPE_OBJECT
)

static void articles_api_finalize(GObject *object) {
    ArticlesApi *self = ARTICLES_API(object);
    g_clear_object(&self->session);
    g_free(self->base_url);
    G_OBJECT_CLASS(articles_api_parent_class)->finalize(object);
}
static void articles_api_class_init(ArticlesApiClass *klass) {
    G_OBJECT_CLASS(klass)->finalize = articles_api_finalize;
}
static void articles_api_init(ArticlesApi *self) {
    self->session = soup_session_new();
}

ArticlesApi *articles_api_new(const gchar *base_url) {
    ArticlesApi *self = g_object_new(ARTICLES_TYPE_API, NULL);
    const gchar *env = g_getenv("ARTICLES_API_BASE_URL");
    self->base_url = g_strdup(
        base_url && *base_url ? base_url
                              : (env && *env ? env : "http://media:8090")
    );
    while (strlen(self->base_url) > 0 &&
           self->base_url[strlen(self->base_url) - 1] == '/') {
        self->base_url[strlen(self->base_url) - 1] = '\0';
    }
    return self;
}

typedef enum { REQUEST_PAGE, REQUEST_DETAIL, REQUEST_IMAGE } RequestKind;
typedef struct {
    RequestKind kind;
} RequestData;

static void parse_response(
    GObject *source,
    GAsyncResult *result,
    gpointer user_data
) {
    GTask *task = G_TASK(user_data);
    RequestData *request = g_task_get_task_data(task);
    g_autoptr(GError) error = NULL;
    g_autoptr(GBytes) bytes =
        soup_session_send_and_read_finish(SOUP_SESSION(source), result, &error);
    if (!bytes) {
        g_task_return_error(task, g_steal_pointer(&error));
        g_object_unref(task);
        return;
    }

    SoupMessage *message = g_object_get_data(G_OBJECT(task), "soup-message");
    guint status = soup_message_get_status(message);
    if (!SOUP_STATUS_IS_SUCCESSFUL(status)) {
        g_task_return_new_error(
            task,
            G_IO_ERROR,
            G_IO_ERROR_FAILED,
            "API request failed (HTTP %u)",
            status
        );
        g_object_unref(task);
        return;
    }

    if (request->kind == REQUEST_IMAGE) {
        GdkTexture *texture = gdk_texture_new_from_bytes(bytes, &error);
        if (texture) {
            g_task_return_pointer(task, texture, g_object_unref);
        } else {
            g_task_return_error(task, g_steal_pointer(&error));
        }
        g_object_unref(task);
        return;
    }

    gsize length = 0;
    const guint8 *data = g_bytes_get_data(bytes, &length);
    GObject *model;
    if (request->kind == REQUEST_PAGE) {
        model = G_OBJECT(api_page_from_json(
            (const gchar *)data,
            length,
            ARTICLE_TYPE_SUMMARY,
            &error
        ));
    } else {
        model = json_gobject_from_data(
            ARTICLE_TYPE_DETAIL,
            (const gchar *)data,
            length,
            &error
        );
    }
    if (!model && !error) {
        g_set_error_literal(
            &error,
            G_IO_ERROR,
            G_IO_ERROR_INVALID_DATA,
            "API response is not a JSON object"
        );
    }
    if (model) {
        g_task_return_pointer(task, model, g_object_unref);
    } else {
        g_task_return_error(task, g_steal_pointer(&error));
    }
    g_object_unref(task);
}

static void send_request(
    ArticlesApi *self,
    const gchar *url,
    RequestKind kind,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data
) {
    GTask *task = g_task_new(self, cancellable, callback, user_data);
    RequestData *data = g_new0(RequestData, 1);
    data->kind = kind;
    g_task_set_task_data(task, data, g_free);

    if (!url || !*url) {
        g_task_return_new_error(
            task,
            G_IO_ERROR,
            G_IO_ERROR_INVALID_ARGUMENT,
            "API request URL is empty"
        );
        g_object_unref(task);
        return;
    }

    SoupMessage *message = soup_message_new("GET", url);
    if (!message) {
        g_task_return_new_error(
            task,
            G_IO_ERROR,
            G_IO_ERROR_INVALID_ARGUMENT,
            "Could not parse API request URL: %s",
            url
        );
        g_object_unref(task);
        return;
    }

    g_object_set_data_full(
        G_OBJECT(task),
        "soup-message",
        message,
        g_object_unref
    );
    soup_session_send_and_read_async(
        self->session,
        message,
        G_PRIORITY_DEFAULT,
        cancellable,
        parse_response,
        task
    );
}

void articles_api_list_async(
    ArticlesApi *self,
    guint page,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data
) {
    g_return_if_fail(ARTICLES_IS_API(self));
    if (page < 1) {
        page = 1;
    }
    g_autofree gchar *url =
        g_strdup_printf("%s/api/articles?page=%u", self->base_url, page);
    send_request(self, url, REQUEST_PAGE, cancellable, callback, user_data);
}
ApiPage *articles_api_list_finish(
    ArticlesApi *self,
    GAsyncResult *result,
    GError **error
) {
    g_return_val_if_fail(g_task_is_valid(result, self), NULL);
    return g_task_propagate_pointer(G_TASK(result), error);
}
void articles_api_get_async(
    ArticlesApi *self,
    const gchar *article_id,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data
) {
    g_return_if_fail(ARTICLES_IS_API(self));
    g_autofree gchar *escaped =
        g_uri_escape_string(article_id ? article_id : "", NULL, TRUE);
    g_autofree gchar *url =
        g_strdup_printf("%s/api/articles/%s", self->base_url, escaped);
    send_request(self, url, REQUEST_DETAIL, cancellable, callback, user_data);
}
ArticleDetail *articles_api_get_finish(
    ArticlesApi *self,
    GAsyncResult *result,
    GError **error
) {
    g_return_val_if_fail(g_task_is_valid(result, self), NULL);
    return g_task_propagate_pointer(G_TASK(result), error);
}
void articles_api_download_image_async(
    ArticlesApi *self,
    const gchar *url,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data
) {
    g_return_if_fail(ARTICLES_IS_API(self));
    send_request(self, url, REQUEST_IMAGE, cancellable, callback, user_data);
}
GdkTexture *articles_api_download_image_finish(
    ArticlesApi *self,
    GAsyncResult *result,
    GError **error
) {
    g_return_val_if_fail(g_task_is_valid(result, self), NULL);
    return g_task_propagate_pointer(G_TASK(result), error);
}
