#ifndef PAGED_LIST_BOX_H
#define PAGED_LIST_BOX_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PAGED_TYPE_LIST_BOX (paged_list_box_get_type())
G_DECLARE_FINAL_TYPE(
    PagedListBox,
    paged_list_box,
    PAGED,
    LIST_BOX,
    GtkBox
)

PagedListBox *paged_list_box_new(void);
void paged_list_box_set_items(
    PagedListBox *self,
    GListModel *items,
    GtkListBoxCreateWidgetFunc create_widget_func,
    gpointer user_data,
    GDestroyNotify user_data_destroy
);
void paged_list_box_set_page(
    PagedListBox *self,
    guint current_page,
    guint total_pages
);
void paged_list_box_set_loading(
    PagedListBox *self,
    gboolean loading
);

G_END_DECLS

#endif
