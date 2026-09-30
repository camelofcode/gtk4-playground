#include <glib.h>
#include <json-glib/json-glib.h>

#include "src/core/helpers/date_helpers.h"
#include "src/core/helpers/json_helpers.h"
#include "src/core/models/api_page.h"
#include "src/core/models/article_comment.h"
#include "src/core/models/article_detail.h"
#include "src/core/models/article_summary.h"

static void test_api_page_deserialization(void) {
    const gchar *json = "{\"currentPage\":1,\"totalPages\":2,\"items\":["
                        "{\"id\":\"article-id\",\"name\":\"Example\","
                        "\"image\":\"https://example.com/image.jpg\","
                        "\"date\":\"2026-09-29T12:00:00Z\"}]}";
    g_autoptr(GError) error = NULL;
    g_autoptr(ApiPage) page =
        api_page_from_json(json, -1, ARTICLE_TYPE_SUMMARY, &error);

    g_assert_no_error(error);
    g_assert_nonnull(page);
    g_assert_cmpuint(api_page_get_current_page(page), ==, 1);
    g_assert_cmpuint(api_page_get_total_pages(page), ==, 2);

    GListModel *items = api_page_get_items(page);
    g_assert_cmpuint(g_list_model_get_n_items(items), ==, 1);
    g_autoptr(ArticleSummary) summary = g_list_model_get_item(items, 0);
    g_assert_cmpstr(article_summary_get_name(summary), ==, "Example");
}

static void test_article_detail_comments_deserialization(void) {
    const gchar *json =
        "{\"id\":\"article-id\",\"name\":\"Example\","
        "\"image\":\"https://example.com/image.jpg\","
        "\"date\":\"2026-09-29T12:00:00Z\","
        "\"description\":\"Details\",\"comments\":["
        "{\"user\":\"Reader\",\"date\":\"2026-09-29T13:00:00Z\","
        "\"text\":\"Great article\"}]}";
    g_autoptr(GError) error = NULL;
    g_autoptr(ArticleDetail) detail = (ArticleDetail *)
        json_gobject_from_data(ARTICLE_TYPE_DETAIL, json, -1, &error);

    g_assert_no_error(error);
    g_assert_nonnull(detail);

    GListModel *comments = article_detail_get_comments(detail);
    g_assert_cmpuint(g_list_model_get_n_items(comments), ==, 1);
    g_autoptr(ArticleComment) comment = g_list_model_get_item(comments, 0);
    g_assert_cmpstr(article_comment_get_user(comment), ==, "Reader");
    g_assert_cmpstr(article_comment_get_text(comment), ==, "Great article");
}

static void test_api_page_rejects_malformed_items(void) {
    const gchar *json = "{\"currentPage\":1,\"totalPages\":2,\"items\":{}}";
    g_autoptr(GError) error = NULL;
    g_autoptr(ApiPage) page =
        api_page_from_json(json, -1, ARTICLE_TYPE_SUMMARY, &error);

    g_assert_null(page);
    g_assert_error(error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA);
}

static void test_api_page_rejects_non_integer_metadata(void) {
    const gchar *json = "{\"currentPage\":\"1\",\"totalPages\":2,\"items\":[]}";
    g_autoptr(GError) error = NULL;
    g_autoptr(ApiPage) page =
        api_page_from_json(json, -1, ARTICLE_TYPE_SUMMARY, &error);

    g_assert_null(page);
    g_assert_error(error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA);
}

static void test_article_detail_rejects_malformed_comments(void) {
    g_autoptr(GError) error = NULL;
    g_autoptr(JsonNode) comments_node =
        json_helpers_from_string("[\"not-an-object\"]", &error);

    g_assert_no_error(error);
    g_assert_nonnull(comments_node);

    g_autoptr(GListStore) comments = json_helpers_deserialize_list(
        comments_node,
        ARTICLE_TYPE_COMMENT,
        &error
    );

    g_assert_null(comments);
    g_assert_error(error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA);
}

static void test_nullable_model_properties(void) {
    g_autoptr(ArticleSummary) summary =
        article_summary_new(NULL, NULL, NULL, NULL);
    g_autoptr(ArticleComment) comment = article_comment_new(NULL, NULL, NULL);
    g_autoptr(ArticleDetail) detail =
        article_detail_new(NULL, NULL, NULL, NULL, NULL);
    g_autofree gchar *summary_id = NULL;
    g_autofree gchar *comment_user = NULL;
    g_autofree gchar *detail_description = NULL;

    GLogLevelFlags previous_fatal_levels = g_log_set_always_fatal(
        G_LOG_FATAL_MASK | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL
    );
    g_object_get(summary, "id", &summary_id, NULL);
    g_object_get(comment, "user", &comment_user, NULL);
    g_object_get(detail, "description", &detail_description, NULL);
    g_log_set_always_fatal(previous_fatal_levels);

    g_assert_null(summary_id);
    g_assert_null(comment_user);
    g_assert_null(detail_description);
}

static void test_date_helper_handles_missing_and_invalid_values(void) {
    g_autofree gchar *missing = date_helpers_format_iso8601(NULL);
    g_autofree gchar *invalid = date_helpers_format_iso8601("not-a-date");

    g_assert_cmpstr(missing, ==, "");
    g_assert_cmpstr(invalid, ==, "not-a-date");
}

int main(
    int argc,
    char **argv
) {
    g_test_init(&argc, &argv, NULL);
    g_test_add_func(
        "/article-models/page-deserialization",
        test_api_page_deserialization
    );
    g_test_add_func(
        "/article-models/detail-comments-deserialization",
        test_article_detail_comments_deserialization
    );
    g_test_add_func(
        "/article-models/page-rejects-malformed-items",
        test_api_page_rejects_malformed_items
    );
    g_test_add_func(
        "/article-models/page-rejects-non-integer-metadata",
        test_api_page_rejects_non_integer_metadata
    );
    g_test_add_func(
        "/article-models/detail-rejects-malformed-comments",
        test_article_detail_rejects_malformed_comments
    );
    g_test_add_func(
        "/article-models/nullable-properties",
        test_nullable_model_properties
    );
    g_test_add_func(
        "/article-models/date-helper-missing-and-invalid",
        test_date_helper_handles_missing_and_invalid_values
    );
    return g_test_run();
}
