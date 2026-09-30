#include "articles_window.h"

#include "src/core/articles_api.h"
#include "src/core/helpers/date_helpers.h"
#include "src/core/models/article_comment.h"
#include "src/core/models/article_summary.h"
#include "src/ui/widgets/article_comment_row.h"
#include "src/ui/widgets/article_picture.h"
#include "src/ui/widgets/article_summary_row.h"
#include "src/ui/widgets/paged_list_box.h"

typedef struct {
    GtkWindow *window;
    GtkStack *stack;
    PagedListBox *paged_list;
    GtkListBox *comments;
    GtkLabel *status;
    GtkLabel *detail_status;
    GtkLabel *detail_title;
    GtkLabel *detail_date;
    GtkLabel *description;
    GtkPicture *detail_picture;
    ArticlesApi *api;
    GCancellable *page_cancellable;
    GCancellable *detail_cancellable;
    guint page_generation;
    guint detail_generation;
    gboolean loaded;
    gboolean closing;
} ArticlesWindow;

typedef struct {
    GWeakRef owner;
    guint generation;
} ArticlesWindowRequest;

static const gchar ARTICLES_WINDOW_DATA_KEY[] = "articles-window-data";

static ArticlesWindowRequest *articles_window_request_new(
    ArticlesWindow *window,
    guint generation
) {
    ArticlesWindowRequest *request = g_new0(ArticlesWindowRequest, 1);
    g_weak_ref_init(&request->owner, window->window);
    request->generation = generation;
    return request;
}

static void articles_window_request_free(ArticlesWindowRequest *request) {
    g_weak_ref_clear(&request->owner);
    g_free(request);
}

static ArticlesWindow *articles_window_request_get_window(
    ArticlesWindowRequest *request,
    GtkWindow **owner
) {
    *owner = g_weak_ref_get(&request->owner);
    if (!*owner) {
        return NULL;
    }
    return g_object_get_data(G_OBJECT(*owner), ARTICLES_WINDOW_DATA_KEY);
}

static void articles_window_data_free(gpointer pointer) {
    ArticlesWindow *window = pointer;
    if (window->page_cancellable) {
        g_cancellable_cancel(window->page_cancellable);
    }
    if (window->detail_cancellable) {
        g_cancellable_cancel(window->detail_cancellable);
    }
    g_clear_object(&window->page_cancellable);
    g_clear_object(&window->detail_cancellable);
    g_clear_object(&window->api);
    g_free(window);
}

static gboolean on_main_window_close_request(
    GtkWindow *main_window,
    ArticlesWindow *window
) {
    (void)main_window;
    window->closing = TRUE;
    if (window->page_cancellable) {
        g_cancellable_cancel(window->page_cancellable);
    }
    if (window->detail_cancellable) {
        g_cancellable_cancel(window->detail_cancellable);
    }
    article_picture_cancel(window->detail_picture);
    gtk_window_destroy(window->window);
    return FALSE;
}

static void article_detail_ready(
    GObject *source,
    GAsyncResult *result,
    gpointer user_data
);

static void on_summary_open_requested(
    ArticleSummaryRow *row,
    ArticleSummary *summary,
    ArticlesWindow *window
) {
    (void)row;
    if (window->closing) {
        return;
    }

    if (window->detail_cancellable) {
        g_cancellable_cancel(window->detail_cancellable);
    }
    g_clear_object(&window->detail_cancellable);
    window->detail_cancellable = g_cancellable_new();
    window->detail_generation++;
    article_picture_cancel(window->detail_picture);
    gtk_picture_set_paintable(window->detail_picture, NULL);
    gtk_label_set_text(window->detail_status, "Loading article…");
    gtk_stack_set_visible_child_name(window->stack, "detail");
    articles_api_get_async(
        window->api,
        article_summary_get_id(summary),
        window->detail_cancellable,
        article_detail_ready,
        articles_window_request_new(window, window->detail_generation)
    );
}

static GtkWidget *create_summary_row(
    gpointer item,
    gpointer user_data
) {
    ArticlesWindow *window = user_data;
    ArticleSummaryRow *row =
        article_summary_row_new(ARTICLE_SUMMARY(item), window->api);
    g_signal_connect(
        row,
        "open-requested",
        G_CALLBACK(on_summary_open_requested),
        window
    );
    return GTK_WIDGET(row);
}

static GtkWidget *create_comment_row(
    gpointer item,
    gpointer user_data
) {
    (void)user_data;
    return GTK_WIDGET(article_comment_row_new(ARTICLE_COMMENT(item)));
}

static void page_loaded(
    GObject *source,
    GAsyncResult *result,
    gpointer user_data
) {
    ArticlesWindowRequest *request = user_data;
    g_autoptr(GtkWindow) owner = NULL;
    ArticlesWindow *window =
        articles_window_request_get_window(request, &owner);
    g_autoptr(GError) error = NULL;
    g_autoptr(ApiPage) page =
        articles_api_list_finish(ARTICLES_API(source), result, &error);

    if (!window || window->closing ||
        request->generation != window->page_generation) {
        articles_window_request_free(request);
        return;
    }

    g_clear_object(&window->page_cancellable);
    if (!page) {
        if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
            gtk_label_set_text(
                window->status,
                error ? error->message : "Could not load articles"
            );
        }
        paged_list_box_set_loading(window->paged_list, FALSE);
        articles_window_request_free(request);
        return;
    }

    paged_list_box_set_items(
        window->paged_list,
        api_page_get_items(page),
        create_summary_row,
        window,
        NULL
    );
    paged_list_box_set_page(
        window->paged_list,
        api_page_get_current_page(page),
        api_page_get_total_pages(page)
    );
    paged_list_box_set_loading(window->paged_list, FALSE);
    gtk_label_set_text(window->status, "");
    window->loaded = TRUE;
    articles_window_request_free(request);
}

static void load_page(
    ArticlesWindow *window,
    guint page
) {
    if (window->closing) {
        return;
    }

    if (window->page_cancellable) {
        g_cancellable_cancel(window->page_cancellable);
    }
    g_clear_object(&window->page_cancellable);
    window->page_cancellable = g_cancellable_new();
    window->page_generation++;
    gtk_label_set_text(window->status, "Loading articles…");
    paged_list_box_set_loading(window->paged_list, TRUE);
    articles_api_list_async(
        window->api,
        page,
        window->page_cancellable,
        page_loaded,
        articles_window_request_new(window, window->page_generation)
    );
}

static void on_page_requested(
    PagedListBox *paged_list,
    guint page,
    ArticlesWindow *window
) {
    (void)paged_list;
    load_page(window, page);
}

static void on_back_clicked(
    GtkButton *button,
    ArticlesWindow *window
) {
    (void)button;
    gtk_stack_set_visible_child_name(window->stack, "list");
}

static void article_detail_ready(
    GObject *source,
    GAsyncResult *result,
    gpointer user_data
) {
    ArticlesWindowRequest *request = user_data;
    g_autoptr(GtkWindow) owner = NULL;
    ArticlesWindow *window =
        articles_window_request_get_window(request, &owner);
    g_autoptr(GError) error = NULL;
    g_autoptr(ArticleDetail) detail =
        articles_api_get_finish(ARTICLES_API(source), result, &error);

    if (!window || window->closing ||
        request->generation != window->detail_generation) {
        articles_window_request_free(request);
        return;
    }

    g_clear_object(&window->detail_cancellable);
    if (!detail) {
        if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
            gtk_label_set_text(
                window->detail_status,
                error ? error->message : "Could not load article"
            );
        }
        articles_window_request_free(request);
        return;
    }

    gtk_label_set_text(window->detail_status, "");
    gtk_label_set_text(window->detail_title, article_detail_get_name(detail));
    g_autofree gchar *date =
        date_helpers_format_iso8601(article_detail_get_date(detail));
    gtk_label_set_text(window->detail_date, date);
    gtk_label_set_text(
        window->description,
        article_detail_get_description(detail)
    );
    article_picture_load(
        window->detail_picture,
        window->api,
        article_detail_get_image(detail)
    );
    gtk_list_box_bind_model(
        window->comments,
        article_detail_get_comments(detail),
        create_comment_row,
        NULL,
        NULL
    );
    articles_window_request_free(request);
}

static void on_open_articles_clicked(
    GtkButton *button,
    ArticlesWindow *window
) {
    (void)button;
    gtk_window_present(window->window);
    if (!window->loaded) {
        load_page(window, 1);
    }
}

void articles_window_init(
    GtkBuilder *builder,
    GtkApplication *app
) {
    ArticlesWindow *window = g_new0(ArticlesWindow, 1);
    GtkWindow *main_window =
        GTK_WINDOW(gtk_builder_get_object(builder, "main_window"));
    window->window =
        GTK_WINDOW(gtk_builder_get_object(builder, "articles_window"));
    window->stack =
        GTK_STACK(gtk_builder_get_object(builder, "articles_stack"));
    window->comments =
        GTK_LIST_BOX(gtk_builder_get_object(builder, "article_comments"));
    window->status =
        GTK_LABEL(gtk_builder_get_object(builder, "articles_status"));
    window->detail_status =
        GTK_LABEL(gtk_builder_get_object(builder, "article_detail_status"));
    window->detail_title =
        GTK_LABEL(gtk_builder_get_object(builder, "article_detail_title"));
    window->detail_date =
        GTK_LABEL(gtk_builder_get_object(builder, "article_detail_date"));
    window->description = GTK_LABEL(
        gtk_builder_get_object(builder, "article_detail_description")
    );
    window->detail_picture =
        GTK_PICTURE(gtk_builder_get_object(builder, "article_detail_picture"));
    window->api = articles_api_new(NULL);
    window->paged_list = paged_list_box_new();
    GtkBox *paged_list_box_slot =
        GTK_BOX(gtk_builder_get_object(builder, "paged_list_box_slot"));
    gtk_box_append(paged_list_box_slot, GTK_WIDGET(window->paged_list));

    gtk_stack_set_visible_child_name(window->stack, "list");
    g_object_set_data_full(
        G_OBJECT(window->window),
        ARTICLES_WINDOW_DATA_KEY,
        window,
        articles_window_data_free
    );
    g_signal_connect(
        window->paged_list,
        "page-requested",
        G_CALLBACK(on_page_requested),
        window
    );
    g_signal_connect(
        gtk_builder_get_object(builder, "article_back_button"),
        "clicked",
        G_CALLBACK(on_back_clicked),
        window
    );
    g_signal_connect(
        main_window,
        "close-request",
        G_CALLBACK(on_main_window_close_request),
        window
    );

    gtk_window_set_application(window->window, app);
    gtk_window_set_transient_for(window->window, main_window);
    gtk_window_set_destroy_with_parent(window->window, TRUE);
    gtk_window_set_resizable(window->window, TRUE);
    g_signal_connect(
        gtk_builder_get_object(builder, "open_articles_window_button"),
        "clicked",
        G_CALLBACK(on_open_articles_clicked),
        window
    );
}
