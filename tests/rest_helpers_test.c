#include <glib.h>

#include "src/core/helpers/json_helpers.h"
#include "src/core/helpers/rest_helpers.h"
#include "src/core/models/article_comment.h"
#include "src/core/models/task.h"
#include <string.h>

typedef struct {
    GMainLoop *loop;
    gboolean completed;
} AsyncTest;

static SoupServer *create_server(SoupServerCallback callback) {
    g_autoptr(GError) error = NULL;
    SoupServer *server = soup_server_new(NULL, NULL);
    soup_server_add_handler(server, NULL, callback, NULL, NULL);
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

static void assert_task_request(
    SoupServerMessage *message,
    const gchar *expected_method
) {
    g_assert_cmpstr(
        soup_server_message_get_method(message),
        ==,
        expected_method
    );

    SoupMessageBody *body = soup_server_message_get_request_body(message);
    g_autoptr(GError) error = NULL;
    g_autoptr(JsonNode) root =
        json_helpers_from_data(body->data, body->length, &error);
    g_assert_no_error(error);
    g_assert_true(JSON_NODE_HOLDS_OBJECT(root));
    JsonObject *object = json_node_get_object(root);
    g_assert_cmpstr(json_object_get_string_member(object, "text"), ==, "Tea");
}

static void set_comment_response(SoupServerMessage *message) {
    const gchar *response =
        "{\"user\":\"Ada\",\"date\":\"2026-10-02\",\"text\":\"Accepted\"}";
    soup_server_message_set_status(message, SOUP_STATUS_OK, NULL);
    soup_server_message_set_response(
        message,
        "application/json",
        SOUP_MEMORY_STATIC,
        response,
        strlen(response)
    );
}

static void success_server_callback(
    SoupServer *server,
    SoupServerMessage *message,
    const gchar *path,
    GHashTable *query,
    gpointer user_data
) {
    (void)server;
    (void)query;
    (void)user_data;

    g_assert_cmpstr(path, ==, "/tasks");
    assert_task_request(message, "POST");
    set_comment_response(message);
}

static void success_response_callback(
    SoupSession *session,
    GObject *response_object,
    const GError *error,
    gpointer user_data
) {
    AsyncTest *test = user_data;
    g_assert_true(SOUP_IS_SESSION(session));
    g_assert_null(error);
    g_assert_true(ARTICLE_IS_COMMENT(response_object));

    ArticleComment *comment = ARTICLE_COMMENT(response_object);
    g_assert_cmpstr(article_comment_get_user(comment), ==, "Ada");
    g_assert_cmpstr(article_comment_get_date(comment), ==, "2026-10-02");
    g_assert_cmpstr(article_comment_get_text(comment), ==, "Accepted");

    test->completed = TRUE;
    g_main_loop_quit(test->loop);
}

static void test_post_serializes_request_and_deserializes_response(void) {
    g_autoptr(SoupServer) server = create_server(success_server_callback);
    g_autofree gchar *base_url = server_url(server);
    g_autofree gchar *url = g_strconcat(base_url, "tasks", NULL);
    g_autoptr(SoupSession) session = soup_session_new();
    g_autoptr(Task) task = task_new("Tea");
    g_autoptr(GMainLoop) loop = g_main_loop_new(NULL, FALSE);
    AsyncTest test = {.loop = loop};

    rest_post_async(
        session,
        url,
        G_OBJECT(task),
        ARTICLE_TYPE_COMMENT,
        NULL,
        success_response_callback,
        &test
    );
    g_main_loop_run(loop);

    g_assert_true(test.completed);
}

static void get_server_callback(
    SoupServer *server,
    SoupServerMessage *message,
    const gchar *path,
    GHashTable *query,
    gpointer user_data
) {
    (void)server;
    (void)path;
    (void)query;
    (void)user_data;

    g_assert_cmpstr(soup_server_message_get_method(message), ==, "GET");
    SoupMessageBody *body = soup_server_message_get_request_body(message);
    g_assert_cmpuint(body->length, ==, 0);

    const gchar *response =
        "{\"user\":\"Grace\",\"date\":\"2026-10-02\",\"text\":\"Found\"}";
    soup_server_message_set_status(message, SOUP_STATUS_OK, NULL);
    soup_server_message_set_response(
        message,
        "application/json",
        SOUP_MEMORY_STATIC,
        response,
        strlen(response)
    );
}

static void get_response_callback(
    SoupSession *session,
    GObject *response_object,
    const GError *error,
    gpointer user_data
) {
    AsyncTest *test = user_data;
    g_assert_true(SOUP_IS_SESSION(session));
    g_assert_null(error);
    g_assert_true(ARTICLE_IS_COMMENT(response_object));
    g_assert_cmpstr(
        article_comment_get_user(ARTICLE_COMMENT(response_object)),
        ==,
        "Grace"
    );

    test->completed = TRUE;
    g_main_loop_quit(test->loop);
}

static void test_get_sends_no_body_and_deserializes_response(void) {
    g_autoptr(SoupServer) server = create_server(get_server_callback);
    g_autofree gchar *url = server_url(server);
    g_autoptr(SoupSession) session = soup_session_new();
    g_autoptr(GMainLoop) loop = g_main_loop_new(NULL, FALSE);
    AsyncTest test = {.loop = loop};

    rest_get_async(
        session,
        url,
        ARTICLE_TYPE_COMMENT,
        NULL,
        get_response_callback,
        &test
    );
    g_main_loop_run(loop);

    g_assert_true(test.completed);
}

static void no_content_server_callback(
    SoupServer *server,
    SoupServerMessage *message,
    const gchar *path,
    GHashTable *query,
    gpointer user_data
) {
    (void)server;
    (void)query;
    (void)user_data;

    g_assert_cmpstr(path, ==, "/tasks/1");
    g_assert_cmpstr(soup_server_message_get_method(message), ==, "DELETE");
    soup_server_message_set_status(message, SOUP_STATUS_NO_CONTENT, NULL);
}

static void no_content_response_callback(
    SoupSession *session,
    GObject *response_object,
    const GError *error,
    gpointer user_data
) {
    AsyncTest *test = user_data;
    g_assert_true(SOUP_IS_SESSION(session));
    g_assert_null(response_object);
    g_assert_null(error);

    test->completed = TRUE;
    g_main_loop_quit(test->loop);
}

static void test_delete_accepts_no_content_response(void) {
    g_autoptr(SoupServer) server = create_server(no_content_server_callback);
    g_autofree gchar *base_url = server_url(server);
    g_autofree gchar *url = g_strconcat(base_url, "tasks/1", NULL);
    g_autoptr(SoupSession) session = soup_session_new();
    g_autoptr(GMainLoop) loop = g_main_loop_new(NULL, FALSE);
    AsyncTest test = {.loop = loop};

    rest_delete_no_response_async(
        session,
        url,
        NULL,
        no_content_response_callback,
        &test
    );
    g_main_loop_run(loop);

    g_assert_true(test.completed);
}

static void update_server_callback(
    SoupServer *server,
    SoupServerMessage *message,
    const gchar *path,
    GHashTable *query,
    gpointer user_data
) {
    (void)server;
    (void)query;
    (void)user_data;

    if (g_str_equal(path, "/tasks/1")) {
        assert_task_request(message, "PUT");
        set_comment_response(message);
        return;
    }

    g_assert_cmpstr(path, ==, "/tasks/2");
    assert_task_request(message, "PATCH");
    soup_server_message_set_status(message, SOUP_STATUS_NO_CONTENT, NULL);
}

static void test_put_serializes_request_and_deserializes_response(void) {
    g_autoptr(SoupServer) server = create_server(update_server_callback);
    g_autofree gchar *base_url = server_url(server);
    g_autofree gchar *url = g_strconcat(base_url, "tasks/1", NULL);
    g_autoptr(SoupSession) session = soup_session_new();
    g_autoptr(Task) task = task_new("Tea");
    g_autoptr(GMainLoop) loop = g_main_loop_new(NULL, FALSE);
    AsyncTest test = {.loop = loop};

    rest_put_async(
        session,
        url,
        G_OBJECT(task),
        ARTICLE_TYPE_COMMENT,
        NULL,
        success_response_callback,
        &test
    );
    g_main_loop_run(loop);

    g_assert_true(test.completed);
}

static void test_patch_serializes_request_and_accepts_no_content(void) {
    g_autoptr(SoupServer) server = create_server(update_server_callback);
    g_autofree gchar *base_url = server_url(server);
    g_autofree gchar *url = g_strconcat(base_url, "tasks/2", NULL);
    g_autoptr(SoupSession) session = soup_session_new();
    g_autoptr(Task) task = task_new("Tea");
    g_autoptr(GMainLoop) loop = g_main_loop_new(NULL, FALSE);
    AsyncTest test = {.loop = loop};

    rest_patch_no_response_async(
        session,
        url,
        G_OBJECT(task),
        NULL,
        no_content_response_callback,
        &test
    );
    g_main_loop_run(loop);

    g_assert_true(test.completed);
}

static void error_server_callback(
    SoupServer *server,
    SoupServerMessage *message,
    const gchar *path,
    GHashTable *query,
    gpointer user_data
) {
    (void)server;
    (void)path;
    (void)query;
    (void)user_data;
    soup_server_message_set_status(message, SOUP_STATUS_BAD_REQUEST, NULL);
}

static void error_response_callback(
    SoupSession *session,
    GObject *response_object,
    const GError *error,
    gpointer user_data
) {
    AsyncTest *test = user_data;
    g_assert_true(SOUP_IS_SESSION(session));
    g_assert_null(response_object);
    g_assert_error(error, G_IO_ERROR, G_IO_ERROR_FAILED);

    test->completed = TRUE;
    g_main_loop_quit(test->loop);
}

static void test_http_error_reaches_callback(void) {
    g_autoptr(SoupServer) server = create_server(error_server_callback);
    g_autofree gchar *url = server_url(server);
    g_autoptr(SoupSession) session = soup_session_new();
    g_autoptr(Task) task = task_new("Tea");
    g_autoptr(GMainLoop) loop = g_main_loop_new(NULL, FALSE);
    AsyncTest test = {.loop = loop};

    rest_post_no_response_async(
        session,
        url,
        G_OBJECT(task),
        NULL,
        error_response_callback,
        &test
    );
    g_main_loop_run(loop);

    g_assert_true(test.completed);
}

static void invalid_json_server_callback(
    SoupServer *server,
    SoupServerMessage *message,
    const gchar *path,
    GHashTable *query,
    gpointer user_data
) {
    (void)server;
    (void)path;
    (void)query;
    (void)user_data;

    const gchar *response = "not JSON";
    soup_server_message_set_status(message, SOUP_STATUS_OK, NULL);
    soup_server_message_set_response(
        message,
        "application/json",
        SOUP_MEMORY_STATIC,
        response,
        strlen(response)
    );
}

static void invalid_json_response_callback(
    SoupSession *session,
    GObject *response_object,
    const GError *error,
    gpointer user_data
) {
    AsyncTest *test = user_data;
    g_assert_true(SOUP_IS_SESSION(session));
    g_assert_null(response_object);
    g_assert_nonnull(error);

    test->completed = TRUE;
    g_main_loop_quit(test->loop);
}

static void test_invalid_json_reaches_callback(void) {
    g_autoptr(SoupServer) server = create_server(invalid_json_server_callback);
    g_autofree gchar *url = server_url(server);
    g_autoptr(SoupSession) session = soup_session_new();
    g_autoptr(Task) task = task_new("Tea");
    g_autoptr(GMainLoop) loop = g_main_loop_new(NULL, FALSE);
    AsyncTest test = {.loop = loop};

    rest_post_async(
        session,
        url,
        G_OBJECT(task),
        ARTICLE_TYPE_COMMENT,
        NULL,
        invalid_json_response_callback,
        &test
    );
    g_main_loop_run(loop);

    g_assert_true(test.completed);
}

static void invalid_arguments_response_callback(
    SoupSession *session,
    GObject *response_object,
    const GError *error,
    gpointer user_data
) {
    AsyncTest *test = user_data;
    g_assert_true(SOUP_IS_SESSION(session));
    g_assert_null(response_object);
    g_assert_error(error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT);

    test->completed = TRUE;
    g_main_loop_quit(test->loop);
}

static void test_invalid_arguments_complete_asynchronously(void) {
    g_autoptr(SoupSession) session = soup_session_new();
    g_autoptr(Task) task = task_new("Tea");
    g_autoptr(GMainLoop) loop = g_main_loop_new(NULL, FALSE);
    AsyncTest test = {.loop = loop};

    rest_post_async(
        session,
        NULL,
        G_OBJECT(task),
        ARTICLE_TYPE_COMMENT,
        NULL,
        invalid_arguments_response_callback,
        &test
    );
    g_assert_false(test.completed);
    g_main_loop_run(loop);

    g_assert_true(test.completed);
}

int main(
    int argc,
    char **argv
) {
    g_test_init(&argc, &argv, NULL);
    g_test_add_func(
        "/rest-helpers/post-json",
        test_post_serializes_request_and_deserializes_response
    );
    g_test_add_func(
        "/rest-helpers/get-json",
        test_get_sends_no_body_and_deserializes_response
    );
    g_test_add_func(
        "/rest-helpers/delete-no-content",
        test_delete_accepts_no_content_response
    );
    g_test_add_func(
        "/rest-helpers/put-json",
        test_put_serializes_request_and_deserializes_response
    );
    g_test_add_func(
        "/rest-helpers/patch-no-content",
        test_patch_serializes_request_and_accepts_no_content
    );
    g_test_add_func(
        "/rest-helpers/http-error",
        test_http_error_reaches_callback
    );
    g_test_add_func(
        "/rest-helpers/invalid-json",
        test_invalid_json_reaches_callback
    );
    g_test_add_func(
        "/rest-helpers/invalid-arguments",
        test_invalid_arguments_complete_asynchronously
    );
    return g_test_run();
}
