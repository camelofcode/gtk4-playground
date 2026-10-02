#include <glib.h>
#include <libsoup/soup.h>

#include "src/core/articles_api.h"
#include "src/core/models/article_summary.h"
#include <string.h>

typedef struct {
    GMainLoop *loop;
    gboolean completed;
} AsyncTest;

static void articles_server_callback(
    SoupServer *server,
    SoupServerMessage *message,
    const gchar *path,
    GHashTable *query,
    gpointer user_data
) {
    (void)server;
    (void)user_data;

    g_assert_cmpstr(soup_server_message_get_method(message), ==, "GET");
    SoupMessageBody *body = soup_server_message_get_request_body(message);
    g_assert_cmpuint(body->length, ==, 0);

    if (g_str_equal(path, "/image")) {
        const gchar *encoded_image =
            "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNk"
            "+A8AAQUBAScY42YAAAAASUVORK5CYII=";
        gsize image_length = 0;
        g_autofree guchar *image =
            g_base64_decode(encoded_image, &image_length);
        soup_server_message_set_status(message, SOUP_STATUS_OK, NULL);
        soup_server_message_set_response(
            message,
            "image/png",
            SOUP_MEMORY_COPY,
            (const gchar *)image,
            image_length
        );
        return;
    }

    const gchar *response = NULL;
    if (g_str_equal(path, "/api/articles")) {
        g_assert_nonnull(query);
        g_assert_cmpstr(g_hash_table_lookup(query, "page"), ==, "2");
        response = "{\"currentPage\":2,\"totalPages\":3,\"items\":["
                   "{\"id\":\"article-id\",\"name\":\"Example\","
                   "\"image\":\"https://example.com/image.jpg\","
                   "\"date\":\"2026-09-29T12:00:00Z\"}]}";
    } else if (g_str_equal(path, "/api/articles/article-id")) {
        response = "{\"id\":\"article-id\",\"name\":\"Example\","
                   "\"image\":\"https://example.com/image.jpg\","
                   "\"date\":\"2026-09-29T12:00:00Z\","
                   "\"description\":\"Details\",\"comments\":[]}";
    } else {
        soup_server_message_set_status(message, SOUP_STATUS_NOT_FOUND, NULL);
        return;
    }

    soup_server_message_set_status(message, SOUP_STATUS_OK, NULL);
    soup_server_message_set_response(
        message,
        "application/json",
        SOUP_MEMORY_STATIC,
        response,
        strlen(response)
    );
}

static SoupServer *create_articles_server(void) {
    g_autoptr(GError) error = NULL;
    SoupServer *server = soup_server_new(NULL, NULL);
    soup_server_add_handler(server, NULL, articles_server_callback, NULL, NULL);
    gboolean listening = soup_server_listen_local(server, 0, 0, &error);
    g_assert_no_error(error);
    g_assert_true(listening);
    return server;
}

static gchar *server_url(SoupServer *server) {
    GSList *uris = soup_server_get_uris(server);
    g_assert_nonnull(uris);
    gchar *url = g_uri_to_string(uris->data);
    g_slist_free_full(uris, (GDestroyNotify)g_uri_unref);
    return url;
}

static void list_ready(
    ArticlesApi *api,
    ApiPage *page,
    const GError *error,
    gpointer user_data
) {
    AsyncTest *test = user_data;

    g_assert_true(ARTICLES_IS_API(api));
    g_assert_null(error);
    g_assert_nonnull(page);
    g_assert_cmpuint(api_page_get_current_page(page), ==, 2);
    g_assert_cmpuint(api_page_get_total_pages(page), ==, 3);
    GListModel *items = api_page_get_items(page);
    g_assert_cmpuint(g_list_model_get_n_items(items), ==, 1);
    g_autoptr(ArticleSummary) summary = g_list_model_get_item(items, 0);
    g_assert_cmpstr(article_summary_get_id(summary), ==, "article-id");

    test->completed = TRUE;
    g_main_loop_quit(test->loop);
}

static void test_list_deserializes_typed_page(void) {
    g_autoptr(SoupServer) server = create_articles_server();
    g_autofree gchar *url = server_url(server);
    g_autoptr(ArticlesApi) api = articles_api_new(url);
    g_autoptr(GMainLoop) loop = g_main_loop_new(NULL, FALSE);
    AsyncTest test = {.loop = loop};

    articles_api_list_async(api, 2, NULL, list_ready, &test);
    g_main_loop_run(loop);

    g_assert_true(test.completed);
}

static void detail_ready(
    ArticlesApi *api,
    ArticleDetail *detail,
    const GError *error,
    gpointer user_data
) {
    AsyncTest *test = user_data;

    g_assert_true(ARTICLES_IS_API(api));
    g_assert_null(error);
    g_assert_nonnull(detail);
    g_assert_cmpstr(article_detail_get_id(detail), ==, "article-id");
    g_assert_cmpstr(article_detail_get_description(detail), ==, "Details");

    test->completed = TRUE;
    g_main_loop_quit(test->loop);
}

static void test_get_deserializes_article_detail(void) {
    g_autoptr(SoupServer) server = create_articles_server();
    g_autofree gchar *url = server_url(server);
    g_autoptr(ArticlesApi) api = articles_api_new(url);
    g_autoptr(GMainLoop) loop = g_main_loop_new(NULL, FALSE);
    AsyncTest test = {.loop = loop};

    articles_api_get_async(api, "article-id", NULL, detail_ready, &test);
    g_main_loop_run(loop);

    g_assert_true(test.completed);
}

static void image_ready(
    ArticlesApi *api,
    GdkTexture *texture,
    const GError *error,
    gpointer user_data
) {
    AsyncTest *test = user_data;

    g_assert_true(ARTICLES_IS_API(api));
    g_assert_null(error);
    g_assert_nonnull(texture);
    g_assert_cmpint(gdk_texture_get_width(texture), ==, 1);
    g_assert_cmpint(gdk_texture_get_height(texture), ==, 1);

    test->completed = TRUE;
    g_main_loop_quit(test->loop);
}

static void test_download_deserializes_texture(void) {
    g_autoptr(SoupServer) server = create_articles_server();
    g_autofree gchar *base_url = server_url(server);
    g_autofree gchar *image_url = g_strconcat(base_url, "image", NULL);
    g_autoptr(ArticlesApi) api = articles_api_new(base_url);
    g_autoptr(GMainLoop) loop = g_main_loop_new(NULL, FALSE);
    AsyncTest test = {.loop = loop};

    articles_api_download_image_async(api, image_url, NULL, image_ready, &test);
    g_main_loop_run(loop);

    g_assert_true(test.completed);
}

static void invalid_page_ready(
    ArticlesApi *api,
    ApiPage *page,
    const GError *error,
    gpointer user_data
) {
    AsyncTest *test = user_data;

    g_assert_true(ARTICLES_IS_API(api));
    g_assert_null(page);
    g_assert_error(error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT);
    test->completed = TRUE;
    g_main_loop_quit(test->loop);
}

static void test_invalid_page_url_completes(void) {
    g_autoptr(ArticlesApi) api = articles_api_new("not a valid URL");
    g_autoptr(GMainLoop) loop = g_main_loop_new(NULL, FALSE);
    AsyncTest test = {.loop = loop};

    articles_api_list_async(api, 1, NULL, invalid_page_ready, &test);
    g_main_loop_run(loop);

    g_assert_true(test.completed);
}

static void invalid_image_ready(
    ArticlesApi *api,
    GdkTexture *texture,
    const GError *error,
    gpointer user_data
) {
    AsyncTest *test = user_data;

    g_assert_true(ARTICLES_IS_API(api));
    g_assert_null(texture);
    g_assert_error(error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT);
    test->completed = TRUE;
    g_main_loop_quit(test->loop);
}

static void test_empty_image_url_completes(void) {
    g_autoptr(ArticlesApi) api = articles_api_new(NULL);
    g_autoptr(GMainLoop) loop = g_main_loop_new(NULL, FALSE);
    AsyncTest test = {.loop = loop};

    articles_api_download_image_async(
        api,
        NULL,
        NULL,
        invalid_image_ready,
        &test
    );
    g_main_loop_run(loop);

    g_assert_true(test.completed);
}

int main(
    int argc,
    char **argv
) {
    g_test_init(&argc, &argv, NULL);
    g_test_add_func(
        "/articles-api/list-deserializes-typed-page",
        test_list_deserializes_typed_page
    );
    g_test_add_func(
        "/articles-api/get-deserializes-detail",
        test_get_deserializes_article_detail
    );
    g_test_add_func(
        "/articles-api/download-deserializes-texture",
        test_download_deserializes_texture
    );
    g_test_add_func(
        "/articles-api/invalid-page-url-completes",
        test_invalid_page_url_completes
    );
    g_test_add_func(
        "/articles-api/empty-image-url-completes",
        test_empty_image_url_completes
    );
    return g_test_run();
}
