#include "src/core/models/task.h"
#include "src/core/task_storage.h"
#include <glib/gstdio.h>

static gchar *path;

static void round_trip(void) {
    g_autoptr(GError) error = NULL;
    g_autoptr(GListStore) tasks = task_storage_load(&error);
    g_assert_no_error(error);
    g_assert_cmpuint(g_list_model_get_n_items(G_LIST_MODEL(tasks)), ==, 0);
    const gchar *texts[] = {"Milk \"quoted\"\n新しい 🐈", "Same", "Same", ""};
    for (guint i = 0; i < G_N_ELEMENTS(texts); i++) {
        g_autoptr(Task) task = task_new(texts[i]);
        task_set_done(task, i == 1);
        g_list_store_append(tasks, task);
    }
    g_assert_true(task_storage_save(G_LIST_MODEL(tasks), &error));
    g_assert_no_error(error);
    g_autoptr(GListStore) loaded = task_storage_load(&error);
    g_assert_no_error(error);
    g_assert_cmpuint(
        g_list_model_get_n_items(G_LIST_MODEL(loaded)),
        ==,
        G_N_ELEMENTS(texts)
    );
    for (guint i = 0; i < G_N_ELEMENTS(texts); i++) {
        g_autoptr(Task) task = g_list_model_get_item(G_LIST_MODEL(loaded), i);
        g_assert_cmpstr(task_get_text(task), ==, texts[i]);
        g_assert_cmpint(task_get_done(task), ==, i == 1);
    }
    g_list_store_remove_all(loaded);
    g_assert_true(task_storage_save(G_LIST_MODEL(loaded), &error));
    g_assert_no_error(error);
    g_clear_object(&tasks);
    tasks = task_storage_load(&error);
    g_assert_no_error(error);
    g_assert_cmpuint(g_list_model_get_n_items(G_LIST_MODEL(tasks)), ==, 0);
    g_remove(path);
}

static void explicit_properties(void) {
    g_autoptr(GError) error = NULL;
    const gchar *json = "[{\"text\":\"Existing\",\"done\":true},"
                        "{\"text\":\"\",\"done\":false},{\"unknown\":42}]";
    g_assert_true(g_file_set_contents(path, json, -1, &error));
    g_assert_no_error(error);
    g_autoptr(GListStore) tasks = task_storage_load(&error);
    g_assert_no_error(error);
    g_assert_cmpuint(g_list_model_get_n_items(G_LIST_MODEL(tasks)), ==, 3);
    for (guint i = 0; i < 3; i++) {
        g_autoptr(Task) task = g_list_model_get_item(G_LIST_MODEL(tasks), i);
        g_assert_cmpstr(task_get_text(task), ==, i == 0 ? "Existing" : "");
        g_assert_cmpint(task_get_done(task), ==, i == 0);
    }
    g_remove(path);
}

static void property_defaults(void) {
    g_autoptr(GError) error = NULL;
    const gchar *json = "[{\"text\":null},{}]";
    g_assert_true(g_file_set_contents(path, json, -1, &error));
    g_assert_no_error(error);
    g_autoptr(GListStore) tasks = task_storage_load(&error);
    g_assert_no_error(error);
    g_assert_nonnull(tasks);
    g_assert_cmpuint(g_list_model_get_n_items(G_LIST_MODEL(tasks)), ==, 2);
    for (guint i = 0; i < 2; i++) {
        g_autoptr(Task) task = g_list_model_get_item(G_LIST_MODEL(tasks), i);
        g_assert_cmpstr(task_get_text(task), ==, "");
        g_assert_false(task_get_done(task));
    }
    g_remove(path);
}

static void invalid_files(void) {
    const gchar *invalid[] = {
        "{",
        "null",
        "42",
        "{}",
        "{\"version\":1,\"tasks\":[]}",
        "[{},null]",
        "[{},42]",
        "[[]]"
    };
    for (guint i = 0; i < G_N_ELEMENTS(invalid); i++) {
        g_autoptr(GError) error = NULL;
        g_assert_true(g_file_set_contents(path, invalid[i], -1, &error));
        g_assert_no_error(error);
        g_autoptr(GListStore) tasks = task_storage_load(&error);
        g_assert_null(tasks);
        g_assert_nonnull(error);
        g_autofree gchar *after = NULL;
        g_assert_true(g_file_get_contents(path, &after, NULL, NULL));
        g_assert_cmpstr(after, ==, invalid[i]);
    }
    g_remove(path);
}

static void io_errors(void) {
    g_assert_cmpint(g_mkdir(path, 0700), ==, 0);
    g_autoptr(GError) error = NULL;
    g_autoptr(GListStore) tasks = task_storage_load(&error);
    g_assert_null(tasks);
    g_assert_nonnull(error);
    g_clear_error(&error);
    tasks = g_list_store_new(TASK_TYPE_ITEM);
    g_assert_false(task_storage_save(G_LIST_MODEL(tasks), &error));
    g_assert_nonnull(error);
    g_assert_cmpint(g_rmdir(path), ==, 0);
}

int main(
    int argc,
    char **argv
) {
    /* Set once before GLib caches the user data directory. */
    g_autofree gchar *directory =
        g_dir_make_tmp("gtk4-storage-test-XXXXXX", NULL);
    g_assert_nonnull(directory);
    g_setenv("XDG_DATA_HOME", directory, TRUE);
    g_test_init(&argc, &argv, NULL);
    path = g_build_filename(directory, "gtk4-playground", "tasks.json", NULL);
    g_autofree gchar *parent = g_path_get_dirname(path);
    g_assert_cmpint(g_mkdir_with_parents(parent, 0700), ==, 0);
    g_test_add_func("/storage/round-trip", round_trip);
    g_test_add_func("/storage/explicit-properties", explicit_properties);
    g_test_add_func("/storage/invalid-files", invalid_files);
    g_test_add_func("/storage/property-defaults", property_defaults);
    g_test_add_func("/storage/io-errors", io_errors);
    int result = g_test_run();
    g_rmdir(parent);
    g_rmdir(directory);
    g_clear_pointer(&path, g_free);
    return result;
}
