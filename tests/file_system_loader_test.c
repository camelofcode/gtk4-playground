#include "src/core/file_system_loader.h"
#include <glib/gstdio.h>

typedef struct {
    GMainLoop *loop;
    FileSystemNode *root;
    GError *error;
} AsyncLoadResult;

static FileSystemNode *find_child(
    FileSystemNode *parent,
    const gchar *name
) {
    const GPtrArray *children = file_system_node_get_children(parent);
    for (guint i = 0; i < children->len; i++) {
        FileSystemNode *child = g_ptr_array_index(children, i);
        if (g_strcmp0(file_system_node_get_name(child), name) == 0) {
            return child;
        }
    }
    return NULL;
}

static gchar *create_test_tree(void) {
    g_autoptr(GError) error = NULL;
    gchar *root = g_dir_make_tmp("file-system-loader-XXXXXX", &error);
    g_assert_no_error(error);
    g_assert_nonnull(root);

    g_autofree gchar *folder = g_build_filename(root, "folder", NULL);
    g_assert_cmpint(g_mkdir(folder, 0700), ==, 0);
    g_autofree gchar *alpha = g_build_filename(root, "alpha.txt", NULL);
    g_autofree gchar *hidden = g_build_filename(root, ".hidden", NULL);
    g_autofree gchar *nested = g_build_filename(folder, "nested.bin", NULL);
    g_assert_true(g_file_set_contents(alpha, "alpha", 5, &error));
    g_assert_no_error(error);
    g_assert_true(g_file_set_contents(hidden, "hidden", 6, &error));
    g_assert_no_error(error);
    g_assert_true(g_file_set_contents(nested, "nested", 6, &error));
    g_assert_no_error(error);
    return root;
}

static void remove_test_tree(const gchar *root) {
    g_autofree gchar *folder = g_build_filename(root, "folder", NULL);
    g_autofree gchar *alpha = g_build_filename(root, "alpha.txt", NULL);
    g_autofree gchar *hidden = g_build_filename(root, ".hidden", NULL);
    g_autofree gchar *nested = g_build_filename(folder, "nested.bin", NULL);
    g_remove(alpha);
    g_remove(hidden);
    g_remove(nested);
    g_rmdir(folder);
    g_rmdir(root);
}

static void assert_test_tree(FileSystemNode *root) {
    g_assert_true(file_system_node_is_directory(root));
    g_assert_cmpuint(file_system_node_get_file_count(root), ==, 3);
    const GPtrArray *children = file_system_node_get_children(root);
    g_assert_cmpuint(children->len, ==, 3);

    FileSystemNode *folder = find_child(root, "folder");
    g_assert_nonnull(folder);
    g_assert_true(file_system_node_is_directory(folder));
    g_assert_cmpuint(file_system_node_get_file_count(folder), ==, 1);
    g_assert_nonnull(find_child(folder, "nested.bin"));

    FileSystemNode *alpha = find_child(root, "alpha.txt");
    g_assert_nonnull(alpha);
    g_assert_false(file_system_node_is_directory(alpha));
    g_assert_cmpuint(file_system_node_get_size(alpha), ==, 5);
    g_assert_nonnull(find_child(root, ".hidden"));
}

static void recursive_load(void) {
    g_autofree gchar *path = create_test_tree();
    g_autoptr(GError) error = NULL;
    g_autoptr(FileSystemNode) root = file_system_loader_load(
        path,
        NULL,
        &error
    );
    g_assert_no_error(error);
    g_assert_nonnull(root);
    assert_test_tree(root);
    remove_test_tree(path);
}

static void on_async_load_finished(
    GObject *source_object,
    GAsyncResult *result,
    gpointer user_data
) {
    (void)source_object;
    AsyncLoadResult *load = user_data;
    load->root = file_system_loader_load_finish(result, &load->error);
    g_main_loop_quit(load->loop);
}

static void asynchronous_load(void) {
    g_autofree gchar *path = create_test_tree();
    AsyncLoadResult load = {
        .loop = g_main_loop_new(NULL, FALSE),
    };
    file_system_loader_load_async(
        path,
        NULL,
        on_async_load_finished,
        &load
    );
    g_main_loop_run(load.loop);
    g_assert_no_error(load.error);
    g_assert_nonnull(load.root);
    assert_test_tree(load.root);
    g_clear_object(&load.root);
    g_clear_error(&load.error);
    g_main_loop_unref(load.loop);
    remove_test_tree(path);
}

int main(
    int argc,
    char **argv
) {
    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/file-system/recursive-load", recursive_load);
    g_test_add_func("/file-system/asynchronous-load", asynchronous_load);
    return g_test_run();
}
