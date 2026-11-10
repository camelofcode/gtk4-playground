#include "articles_api.h"

#include "helpers/rest_helpers.h"
#include "helpers/texture_helpers.h"
#include "models/article_summary.h"
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
    const gchar *environment_url = g_getenv("ARTICLES_API_BASE_URL");
    self->base_url = g_strdup(
        base_url && *base_url
            ? base_url
            : (environment_url && *environment_url ? environment_url
                                                   : "http://localhost:8090")
    );
    while (strlen(self->base_url) > 0 &&
           self->base_url[strlen(self->base_url) - 1] == '/') {
        self->base_url[strlen(self->base_url) - 1] = '\0';
    }
    return self;
}

typedef enum {
    ARTICLES_API_RESPONSE_PAGE,
    ARTICLES_API_RESPONSE_DETAIL,
    ARTICLES_API_RESPONSE_IMAGE,
} ArticlesApiResponseKind;

typedef union {
    ArticlesApiListCallback list;
    ArticlesApiDetailCallback detail;
    ArticlesApiImageCallback image;
} ArticlesApiResponseCallback;

typedef struct {
    ArticlesApi *api;
    ArticlesApiResponseKind response_kind;
    ArticlesApiResponseCallback callback;
    gpointer user_data;
} ArticlesApiRequest;

static ArticlesApiRequest *articles_api_request_new(
    ArticlesApi *self,
    ArticlesApiResponseKind response_kind,
    gpointer user_data
) {
    ArticlesApiRequest *request = g_new0(ArticlesApiRequest, 1);
    request->api = g_object_ref(self);
    request->response_kind = response_kind;
    request->user_data = user_data;
    return request;
}

static void articles_api_request_free(ArticlesApiRequest *request) {
    g_clear_object(&request->api);
    g_free(request);
}

static void rest_response_ready(
    SoupSession *session,
    GObject *response_object,
    const GError *error,
    gpointer user_data
) {
    (void)session;

    ArticlesApiRequest *request = user_data;
    switch (request->response_kind) {
    case ARTICLES_API_RESPONSE_PAGE:
        request->callback.list(
            request->api,
            response_object ? API_PAGE(response_object) : NULL,
            error,
            request->user_data
        );
        break;
    case ARTICLES_API_RESPONSE_DETAIL:
        request->callback.detail(
            request->api,
            response_object ? ARTICLE_DETAIL(response_object) : NULL,
            error,
            request->user_data
        );
        break;
    case ARTICLES_API_RESPONSE_IMAGE:
        request->callback.image(
            request->api,
            response_object ? GDK_TEXTURE(response_object) : NULL,
            error,
            request->user_data
        );
        break;
    }
    articles_api_request_free(request);
}

void articles_api_list_async(
    ArticlesApi *self,
    guint page,
    GCancellable *cancellable,
    ArticlesApiListCallback callback,
    gpointer user_data
) {
    g_return_if_fail(ARTICLES_IS_API(self));
    g_return_if_fail(callback != NULL);
    if (page < 1) {
        page = 1;
    }
    g_autofree gchar *url =
        g_strdup_printf("%s/api/articles?page=%u", self->base_url, page);
    ArticlesApiRequest *request =
        articles_api_request_new(self, ARTICLES_API_RESPONSE_PAGE, user_data);
    request->callback.list = callback;
    rest_get_with_response_deserializer_async(
        self->session,
        url,
        ARTICLE_TYPE_SUMMARY,
        api_page_deserialize_response,
        cancellable,
        rest_response_ready,
        request
    );
}

void articles_api_get_async(
    ArticlesApi *self,
    const gchar *article_id,
    GCancellable *cancellable,
    ArticlesApiDetailCallback callback,
    gpointer user_data
) {
    g_return_if_fail(ARTICLES_IS_API(self));
    g_return_if_fail(callback != NULL);
    g_autofree gchar *escaped =
        g_uri_escape_string(article_id ? article_id : "", NULL, TRUE);
    g_autofree gchar *url =
        g_strdup_printf("%s/api/articles/%s", self->base_url, escaped);
    ArticlesApiRequest *request =
        articles_api_request_new(self, ARTICLES_API_RESPONSE_DETAIL, user_data);
    request->callback.detail = callback;
    rest_get_async(
        self->session,
        url,
        ARTICLE_TYPE_DETAIL,
        cancellable,
        rest_response_ready,
        request
    );
}

void articles_api_download_image_async(
    ArticlesApi *self,
    const gchar *url,
    GCancellable *cancellable,
    ArticlesApiImageCallback callback,
    gpointer user_data
) {
    g_return_if_fail(ARTICLES_IS_API(self));
    g_return_if_fail(callback != NULL);
    ArticlesApiRequest *request =
        articles_api_request_new(self, ARTICLES_API_RESPONSE_IMAGE, user_data);
    request->callback.image = callback;
    rest_get_with_response_deserializer_async(
        self->session,
        url,
        GDK_TYPE_TEXTURE,
        texture_helpers_deserialize_response,
        cancellable,
        rest_response_ready,
        request
    );
}
