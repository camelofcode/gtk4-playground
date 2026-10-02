#include "articles_list_page.h"

#include "src/ui/widgets/article_summary_row.h"
#include "src/ui/widgets/paged_list_box.h"

struct _ArticlesListPage {
    GObject parent_instance;

    GtkWidget *root;
    GtkLabel *status;
    PagedListBox *paged_list;
    ArticlesApi *api;
    GCancellable *cancellable;
    ArticlesListPageOpenCallback open_callback;
    gpointer callback_data;
    guint generation;
    gboolean loaded;
    gboolean closing;
};

typedef struct {
    ArticlesListPage *page;
    guint generation;
} ArticlesListPageRequest;

G_DEFINE_TYPE(
    ArticlesListPage,
    articles_list_page,
    G_TYPE_OBJECT
)

static ArticlesListPageRequest *
articles_list_page_request_new(ArticlesListPage *self) {
    ArticlesListPageRequest *request = g_new0(ArticlesListPageRequest, 1);
    request->page = g_object_ref(self);
    request->generation = self->generation;
    return request;
}

static void articles_list_page_request_free(ArticlesListPageRequest *request) {
    g_clear_object(&request->page);
    g_free(request);
}

static void on_summary_open_requested(
    ArticleSummaryRow *row,
    ArticleSummary *summary,
    ArticlesListPage *self
) {
    (void)row;

    if (!self->closing) {
        self->open_callback(self, summary, self->callback_data);
    }
}

static GtkWidget *create_summary_row(
    gpointer item,
    gpointer user_data
) {
    ArticlesListPage *self = ARTICLES_LIST_PAGE(user_data);
    ArticleSummaryRow *row =
        article_summary_row_new(ARTICLE_SUMMARY(item), self->api);
    g_signal_connect(
        row,
        "open-requested",
        G_CALLBACK(on_summary_open_requested),
        self
    );
    return GTK_WIDGET(row);
}

static void page_loaded(
    ArticlesApi *api,
    ApiPage *page,
    const GError *error,
    gpointer user_data
) {
    (void)api;

    ArticlesListPageRequest *request = user_data;
    ArticlesListPage *self = request->page;

    if (self->closing || request->generation != self->generation) {
        articles_list_page_request_free(request);
        return;
    }

    g_clear_object(&self->cancellable);
    if (!page) {
        if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
            gtk_label_set_text(
                self->status,
                error ? error->message : "Could not load articles"
            );
        }
        paged_list_box_set_loading(self->paged_list, FALSE);
        articles_list_page_request_free(request);
        return;
    }

    paged_list_box_set_items(
        self->paged_list,
        api_page_get_items(page),
        create_summary_row,
        self,
        NULL
    );
    paged_list_box_set_page(
        self->paged_list,
        api_page_get_current_page(page),
        api_page_get_total_pages(page)
    );
    paged_list_box_set_loading(self->paged_list, FALSE);
    gtk_label_set_text(self->status, "");
    self->loaded = TRUE;
    articles_list_page_request_free(request);
}

static void load_page(
    ArticlesListPage *self,
    guint page
) {
    if (self->closing) {
        return;
    }

    if (self->cancellable) {
        g_cancellable_cancel(self->cancellable);
    }
    g_clear_object(&self->cancellable);
    self->cancellable = g_cancellable_new();
    self->generation++;
    gtk_label_set_text(self->status, "Loading articles…");
    paged_list_box_set_loading(self->paged_list, TRUE);
    articles_api_list_async(
        self->api,
        page,
        self->cancellable,
        page_loaded,
        articles_list_page_request_new(self)
    );
}

static void on_page_requested(
    PagedListBox *paged_list,
    guint page,
    ArticlesListPage *self
) {
    (void)paged_list;
    load_page(self, page);
}

static void articles_list_page_dispose(GObject *object) {
    ArticlesListPage *self = ARTICLES_LIST_PAGE(object);

    articles_list_page_cancel(self);
    g_clear_object(&self->api);
    g_clear_object(&self->root);
    self->status = NULL;
    self->paged_list = NULL;

    G_OBJECT_CLASS(articles_list_page_parent_class)->dispose(object);
}

static void articles_list_page_class_init(ArticlesListPageClass *class) {
    GObjectClass *object_class = G_OBJECT_CLASS(class);
    object_class->dispose = articles_list_page_dispose;
}

static void articles_list_page_init(ArticlesListPage *self) { (void)self; }

ArticlesListPage *articles_list_page_new(
    GtkBuilder *builder,
    ArticlesApi *api,
    ArticlesListPageOpenCallback open_callback,
    gpointer user_data
) {
    g_return_val_if_fail(GTK_IS_BUILDER(builder), NULL);
    g_return_val_if_fail(ARTICLES_IS_API(api), NULL);
    g_return_val_if_fail(open_callback != NULL, NULL);

    GtkWidget *root =
        GTK_WIDGET(gtk_builder_get_object(builder, "articles_list_view"));
    GtkLabel *status =
        GTK_LABEL(gtk_builder_get_object(builder, "articles_status"));
    GtkBox *paged_list_box_slot =
        GTK_BOX(gtk_builder_get_object(builder, "paged_list_box_slot"));
    g_return_val_if_fail(GTK_IS_WIDGET(root), NULL);
    g_return_val_if_fail(GTK_IS_LABEL(status), NULL);
    g_return_val_if_fail(GTK_IS_BOX(paged_list_box_slot), NULL);

    ArticlesListPage *self = g_object_new(ARTICLES_TYPE_LIST_PAGE, NULL);
    self->root = g_object_ref(root);
    self->status = status;
    self->api = g_object_ref(api);
    self->open_callback = open_callback;
    self->callback_data = user_data;
    self->paged_list = paged_list_box_new();
    gtk_box_append(paged_list_box_slot, GTK_WIDGET(self->paged_list));
    g_signal_connect(
        self->paged_list,
        "page-requested",
        G_CALLBACK(on_page_requested),
        self
    );

    return self;
}

void articles_list_page_load_initial(ArticlesListPage *self) {
    g_return_if_fail(ARTICLES_IS_LIST_PAGE(self));

    if (!self->loaded) {
        load_page(self, 1);
    }
}

void articles_list_page_cancel(ArticlesListPage *self) {
    g_return_if_fail(ARTICLES_IS_LIST_PAGE(self));

    self->closing = TRUE;
    self->generation++;
    if (self->cancellable) {
        g_cancellable_cancel(self->cancellable);
    }
    g_clear_object(&self->cancellable);
}
