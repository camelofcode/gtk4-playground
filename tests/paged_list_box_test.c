#include <gtk/gtk.h>

#include "src/ui/widgets/paged_list_box.h"

static void record_requested_page(
    PagedListBox *paged_list,
    guint page,
    guint *requested_page
) {
    (void)paged_list;
    *requested_page = page;
}

static void test_page_controls(void) {
    g_autoptr(PagedListBox) paged_list = paged_list_box_new();
    g_object_ref_sink(paged_list);
    GtkButton *previous_button = GTK_BUTTON(gtk_widget_get_template_child(
        GTK_WIDGET(paged_list),
        PAGED_TYPE_LIST_BOX,
        "previous_button"
    ));
    GtkButton *next_button = GTK_BUTTON(gtk_widget_get_template_child(
        GTK_WIDGET(paged_list),
        PAGED_TYPE_LIST_BOX,
        "next_button"
    ));
    guint requested_page = 0;
    g_signal_connect(
        paged_list,
        "page-requested",
        G_CALLBACK(record_requested_page),
        &requested_page
    );

    paged_list_box_set_page(paged_list, 2, 3);
    g_assert_true(gtk_widget_get_sensitive(GTK_WIDGET(previous_button)));
    g_assert_true(gtk_widget_get_sensitive(GTK_WIDGET(next_button)));

    g_signal_emit_by_name(previous_button, "clicked");
    g_assert_cmpuint(requested_page, ==, 1);

    requested_page = 0;
    g_signal_emit_by_name(next_button, "clicked");
    g_assert_cmpuint(requested_page, ==, 3);

    paged_list_box_set_loading(paged_list, TRUE);
    g_assert_false(gtk_widget_get_sensitive(GTK_WIDGET(previous_button)));
    g_assert_false(gtk_widget_get_sensitive(GTK_WIDGET(next_button)));

    paged_list_box_set_loading(paged_list, FALSE);
    g_assert_true(gtk_widget_get_sensitive(GTK_WIDGET(previous_button)));
    g_assert_true(gtk_widget_get_sensitive(GTK_WIDGET(next_button)));
}

int main(
    int argc,
    char **argv
) {
    if (!gtk_init_check()) {
        return 77;
    }
    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/paged-list-box/page-controls", test_page_controls);
    return g_test_run();
}
