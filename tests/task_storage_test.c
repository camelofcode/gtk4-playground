#include "src/core/task_storage.h"
#include <glib/gstdio.h>

static void round_trip(void) {
    g_autoptr(GError) error = NULL;
    g_autofree gchar *directory =
        g_dir_make_tmp("gtk4-tasks-test-XXXXXX", &error);
    g_assert_no_error(error);
    g_autofree gchar *path = g_build_filename(directory, "tasks.json", NULL);
    g_autoptr(GListStore) tasks = task_storage_load(path, &error);
    g_assert_no_error(error);
    g_assert_cmpuint(g_list_model_get_n_items(G_LIST_MODEL(tasks)), ==, 0);
    const gchar *texts[] = {"Milk \"quoted\"\n新しい 🐈", "Same", "Same"};
    for (guint i = 0; i < G_N_ELEMENTS(texts); i++) {
        g_autoptr(Task) task = task_new(texts[i]);
        task_set_done(task, i == 1);
        g_list_store_append(tasks, task);
    }
    g_assert_true(task_storage_save(G_LIST_MODEL(tasks), path, &error));
    g_assert_no_error(error);
    g_autoptr(GListStore) loaded = task_storage_load(path, &error);
    g_assert_no_error(error);
    g_assert_cmpuint(g_list_model_get_n_items(G_LIST_MODEL(loaded)), ==, 3);
    for (guint i = 0; i < G_N_ELEMENTS(texts); i++) {
        g_autoptr(Task) task = g_list_model_get_item(G_LIST_MODEL(loaded), i);
        g_assert_cmpstr(task_get_text(task), ==, texts[i]);
        g_assert_cmpint(task_get_done(task), ==, i == 1);
    }
    g_list_store_remove_all(loaded);
    g_assert_true(task_storage_save(G_LIST_MODEL(loaded), path, &error));
    g_assert_no_error(error);
    g_clear_object(&tasks);
    tasks = task_storage_load(path, &error);
    g_assert_no_error(error);
    g_assert_cmpuint(g_list_model_get_n_items(G_LIST_MODEL(tasks)), ==, 0);
    g_remove(path);
    g_rmdir(directory);
}

static void invalid_files(void) {
    const gchar *invalid[] = {
        "{",
        "null",
        "[]",
        "{}",
        "{\"version\":2,\"tasks\":[]}",
        "{\"version\":1,\"tasks\":[{\"text\":\"valid\",\"done\":true},{"
        "\"text\":4,\"done\":true}]}",
        "{\"version\":1,\"tasks\":[{\"text\":\"x\",\"done\":\"false\"}]}",
        "{\"version\":1,\"tasks\":[{\"text\":null,\"done\":false}]}"
    };
    g_autofree gchar *directory =
        g_dir_make_tmp("gtk4-tasks-invalid-XXXXXX", NULL);
    g_autofree gchar *path = g_build_filename(directory, "tasks.json", NULL);
    for (guint i = 0; i < G_N_ELEMENTS(invalid); i++) {
        g_autoptr(GError) error = NULL;
        g_assert_true(g_file_set_contents(path, invalid[i], -1, &error));
        g_assert_no_error(error);
        g_autoptr(GListStore) tasks = task_storage_load(path, &error);
        g_assert_null(tasks);
        g_assert_nonnull(error);
        g_autofree gchar *after = NULL;
        g_assert_true(g_file_get_contents(path, &after, NULL, NULL));
        g_assert_cmpstr(after, ==, invalid[i]);
    }
    g_remove(path);
    g_rmdir(directory);
}

static void io_errors(void) {
    g_autofree gchar *directory = g_dir_make_tmp("gtk4-tasks-io-XXXXXX", NULL);
    g_autoptr(GError) error = NULL;
    g_autoptr(GListStore) tasks = task_storage_load(directory, &error);
    g_assert_null(tasks);
    g_assert_nonnull(error);
    g_clear_error(&error);
    tasks = g_list_store_new(TASK_TYPE_ITEM);
    g_assert_false(task_storage_save(G_LIST_MODEL(tasks), directory, &error));
    g_assert_nonnull(error);
    g_rmdir(directory);
}

int main(
    int argc,
    char **argv
) {
    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/storage/round-trip", round_trip);
    g_test_add_func("/storage/invalid-files", invalid_files);
    g_test_add_func("/storage/io-errors", io_errors);
    return g_test_run();
}
