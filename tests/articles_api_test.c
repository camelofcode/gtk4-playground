#include <glib.h>

#include "src/core/articles_api.h"

typedef struct {
    GMainLoop *loop;
    gboolean completed;
} AsyncTest;

static void invalid_page_ready(
    GObject *source,
    GAsyncResult *result,
    gpointer user_data
) {
    AsyncTest *test = user_data;
    g_autoptr(GError) error = NULL;
    g_autoptr(ApiPage) page =
        articles_api_list_finish(ARTICLES_API(source), result, &error);

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
    GObject *source,
    GAsyncResult *result,
    gpointer user_data
) {
    AsyncTest *test = user_data;
    g_autoptr(GError) error = NULL;
    g_autoptr(GdkTexture) texture = articles_api_download_image_finish(
        ARTICLES_API(source),
        result,
        &error
    );

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
        "/articles-api/invalid-page-url-completes",
        test_invalid_page_url_completes
    );
    g_test_add_func(
        "/articles-api/empty-image-url-completes",
        test_empty_image_url_completes
    );
    return g_test_run();
}
