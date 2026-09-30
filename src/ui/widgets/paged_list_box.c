#include "paged_list_box.h"

enum { PAGE_REQUESTED, N_SIGNALS };

struct _PagedListBox {
    GtkBox parent_instance;

    GtkListBox *list;
    GtkButton *previous_button;
    GtkButton *next_button;
    GtkLabel *page_label;

    guint current_page;
    guint total_pages;
    gboolean loading;
};

static guint signals[N_SIGNALS];

G_DEFINE_TYPE(
    PagedListBox,
    paged_list_box,
    GTK_TYPE_BOX
)

static void paged_list_box_update_controls(PagedListBox *self) {
    g_autofree gchar *label =
        g_strdup_printf("Page %u of %u", self->current_page, self->total_pages);
    gtk_label_set_text(self->page_label, label);
    gtk_widget_set_sensitive(
        GTK_WIDGET(self->previous_button),
        !self->loading && self->current_page > 1
    );
    gtk_widget_set_sensitive(
        GTK_WIDGET(self->next_button),
        !self->loading && self->current_page < self->total_pages
    );
}

static void paged_list_box_on_previous_clicked(
    GtkButton *button,
    PagedListBox *self
) {
    (void)button;
    if (!self->loading && self->current_page > 1) {
        g_signal_emit(self, signals[PAGE_REQUESTED], 0, self->current_page - 1);
    }
}

static void paged_list_box_on_next_clicked(
    GtkButton *button,
    PagedListBox *self
) {
    (void)button;
    if (!self->loading && self->current_page < self->total_pages) {
        g_signal_emit(self, signals[PAGE_REQUESTED], 0, self->current_page + 1);
    }
}

static void paged_list_box_dispose(GObject *object) {
    gtk_widget_dispose_template(GTK_WIDGET(object), PAGED_TYPE_LIST_BOX);
    G_OBJECT_CLASS(paged_list_box_parent_class)->dispose(object);
}

static void paged_list_box_class_init(PagedListBoxClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    object_class->dispose = paged_list_box_dispose;

    signals[PAGE_REQUESTED] = g_signal_new(
        "page-requested",
        G_TYPE_FROM_CLASS(klass),
        G_SIGNAL_RUN_LAST,
        0,
        NULL,
        NULL,
        NULL,
        G_TYPE_NONE,
        1,
        G_TYPE_UINT
    );

    gtk_widget_class_set_template_from_resource(
        widget_class,
        "/com/camelofdoom/gtk4-playground/ui/widgets/paged-list-box.ui"
    );
    gtk_widget_class_bind_template_child(widget_class, PagedListBox, list);
    gtk_widget_class_bind_template_child(
        widget_class,
        PagedListBox,
        previous_button
    );
    gtk_widget_class_bind_template_child(
        widget_class,
        PagedListBox,
        next_button
    );
    gtk_widget_class_bind_template_child(
        widget_class,
        PagedListBox,
        page_label
    );
}

static void paged_list_box_init(PagedListBox *self) {
    gtk_widget_init_template(GTK_WIDGET(self));
    self->current_page = 1;
    self->total_pages = 1;

    g_signal_connect(
        self->previous_button,
        "clicked",
        G_CALLBACK(paged_list_box_on_previous_clicked),
        self
    );
    g_signal_connect(
        self->next_button,
        "clicked",
        G_CALLBACK(paged_list_box_on_next_clicked),
        self
    );
    paged_list_box_update_controls(self);
}

PagedListBox *paged_list_box_new(void) {
    return g_object_new(PAGED_TYPE_LIST_BOX, NULL);
}

void paged_list_box_set_items(
    PagedListBox *self,
    GListModel *items,
    GtkListBoxCreateWidgetFunc create_widget_func,
    gpointer user_data,
    GDestroyNotify user_data_destroy
) {
    g_return_if_fail(PAGED_IS_LIST_BOX(self));
    g_return_if_fail(items == NULL || G_IS_LIST_MODEL(items));

    gtk_list_box_bind_model(
        self->list,
        items,
        create_widget_func,
        user_data,
        user_data_destroy
    );
}

void paged_list_box_set_page(
    PagedListBox *self,
    guint current_page,
    guint total_pages
) {
    g_return_if_fail(PAGED_IS_LIST_BOX(self));

    self->total_pages = MAX(total_pages, 1);
    self->current_page = CLAMP(current_page, 1, self->total_pages);
    paged_list_box_update_controls(self);
}

void paged_list_box_set_loading(
    PagedListBox *self,
    gboolean loading
) {
    g_return_if_fail(PAGED_IS_LIST_BOX(self));

    self->loading = loading;
    paged_list_box_update_controls(self);
}
