#include "articles_window.h"

#include "articles_detail_page.h"
#include "articles_list_page.h"
#include "src/core/articles_api.h"

typedef struct {
    GtkWindow *window;
    GtkStack *stack;
    ArticlesApi *api;
    ArticlesListPage *list_page;
    ArticlesDetailPage *detail_page;
    gboolean closing;
} ArticlesWindow;

static const gchar ARTICLES_WINDOW_DATA_KEY[] = "articles-window-data";

static void articles_window_data_free(gpointer pointer) {
    ArticlesWindow *self = pointer;

    articles_list_page_cancel(self->list_page);
    articles_detail_page_cancel(self->detail_page);
    g_clear_object(&self->list_page);
    g_clear_object(&self->detail_page);
    g_clear_object(&self->api);
    g_free(self);
}

static gboolean on_main_window_close_request(
    GtkWindow *main_window,
    ArticlesWindow *self
) {
    (void)main_window;

    self->closing = TRUE;
    articles_list_page_cancel(self->list_page);
    articles_detail_page_cancel(self->detail_page);
    gtk_window_destroy(self->window);
    return FALSE;
}

static void on_article_open_requested(
    ArticlesListPage *list_page,
    ArticleSummary *summary,
    gpointer user_data
) {
    (void)list_page;

    ArticlesWindow *self = user_data;
    if (self->closing) {
        return;
    }

    articles_detail_page_load(
        self->detail_page,
        article_summary_get_id(summary)
    );
    gtk_stack_set_visible_child_name(self->stack, "detail");
}

static void on_article_back_requested(
    ArticlesDetailPage *detail_page,
    gpointer user_data
) {
    (void)detail_page;

    ArticlesWindow *self = user_data;
    if (!self->closing) {
        gtk_stack_set_visible_child_name(self->stack, "list");
    }
}

static void on_open_articles_clicked(
    GtkButton *button,
    ArticlesWindow *self
) {
    (void)button;

    gtk_window_present(self->window);
    articles_list_page_load_initial(self->list_page);
}

void articles_window_init(
    GtkBuilder *builder,
    GtkApplication *app
) {
    g_return_if_fail(GTK_IS_BUILDER(builder));
    g_return_if_fail(GTK_IS_APPLICATION(app));

    GtkWindow *main_window =
        GTK_WINDOW(gtk_builder_get_object(builder, "main_window"));
    GtkWindow *articles_window =
        GTK_WINDOW(gtk_builder_get_object(builder, "articles_window"));
    GtkStack *stack =
        GTK_STACK(gtk_builder_get_object(builder, "articles_stack"));
    GtkButton *open_button = GTK_BUTTON(
        gtk_builder_get_object(builder, "open_articles_window_button")
    );
    g_return_if_fail(GTK_IS_WINDOW(main_window));
    g_return_if_fail(GTK_IS_WINDOW(articles_window));
    g_return_if_fail(GTK_IS_STACK(stack));
    g_return_if_fail(GTK_IS_BUTTON(open_button));

    ArticlesWindow *self = g_new0(ArticlesWindow, 1);
    self->window = articles_window;
    self->stack = stack;
    self->api = articles_api_new(NULL);
    self->list_page = articles_list_page_new(
        builder,
        self->api,
        on_article_open_requested,
        self
    );
    self->detail_page = articles_detail_page_new(
        builder,
        self->api,
        on_article_back_requested,
        self
    );

    gtk_stack_set_visible_child_name(self->stack, "list");
    g_object_set_data_full(
        G_OBJECT(self->window),
        ARTICLES_WINDOW_DATA_KEY,
        self,
        articles_window_data_free
    );
    g_signal_connect(
        main_window,
        "close-request",
        G_CALLBACK(on_main_window_close_request),
        self
    );
    g_signal_connect(
        open_button,
        "clicked",
        G_CALLBACK(on_open_articles_clicked),
        self
    );

    gtk_window_set_application(self->window, app);
    gtk_window_set_transient_for(self->window, main_window);
    gtk_window_set_destroy_with_parent(self->window, TRUE);
    gtk_window_set_resizable(self->window, TRUE);
}
