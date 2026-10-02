#include "articles_detail_page.h"

#include "src/core/helpers/date_helpers.h"
#include "src/core/models/article_comment.h"
#include "src/ui/widgets/article_comment_row.h"
#include "src/ui/widgets/article_picture.h"

struct _ArticlesDetailPage {
    GObject parent_instance;

    GtkWidget *root;
    GtkLabel *status;
    GtkLabel *title;
    GtkLabel *date;
    GtkLabel *description;
    GtkPicture *picture;
    GtkListBox *comments;
    ArticlesApi *api;
    GCancellable *cancellable;
    ArticlesDetailPageBackCallback back_callback;
    gpointer callback_data;
    guint generation;
    gboolean closing;
};

typedef struct {
    ArticlesDetailPage *page;
    guint generation;
} ArticlesDetailPageRequest;

G_DEFINE_TYPE(
    ArticlesDetailPage,
    articles_detail_page,
    G_TYPE_OBJECT
)

static ArticlesDetailPageRequest *
articles_detail_page_request_new(ArticlesDetailPage *self) {
    ArticlesDetailPageRequest *request = g_new0(ArticlesDetailPageRequest, 1);
    request->page = g_object_ref(self);
    request->generation = self->generation;
    return request;
}

static void
articles_detail_page_request_free(ArticlesDetailPageRequest *request) {
    g_clear_object(&request->page);
    g_free(request);
}

static GtkWidget *create_comment_row(
    gpointer item,
    gpointer user_data
) {
    (void)user_data;
    return GTK_WIDGET(article_comment_row_new(ARTICLE_COMMENT(item)));
}

static void article_detail_ready(
    ArticlesApi *api,
    ArticleDetail *detail,
    const GError *error,
    gpointer user_data
) {
    (void)api;

    ArticlesDetailPageRequest *request = user_data;
    ArticlesDetailPage *self = request->page;

    if (self->closing || request->generation != self->generation) {
        articles_detail_page_request_free(request);
        return;
    }

    g_clear_object(&self->cancellable);
    if (!detail) {
        if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
            gtk_label_set_text(
                self->status,
                error ? error->message : "Could not load article"
            );
        }
        articles_detail_page_request_free(request);
        return;
    }

    gtk_label_set_text(self->status, "");
    gtk_label_set_text(self->title, article_detail_get_name(detail));
    g_autofree gchar *date =
        date_helpers_format_iso8601(article_detail_get_date(detail));
    gtk_label_set_text(self->date, date);
    gtk_label_set_text(
        self->description,
        article_detail_get_description(detail)
    );
    article_picture_load(
        self->picture,
        self->api,
        article_detail_get_image(detail)
    );
    gtk_list_box_bind_model(
        self->comments,
        article_detail_get_comments(detail),
        create_comment_row,
        NULL,
        NULL
    );
    articles_detail_page_request_free(request);
}

static void on_back_clicked(
    GtkButton *button,
    ArticlesDetailPage *self
) {
    (void)button;

    if (!self->closing) {
        self->back_callback(self, self->callback_data);
    }
}

static void articles_detail_page_dispose(GObject *object) {
    ArticlesDetailPage *self = ARTICLES_DETAIL_PAGE(object);

    articles_detail_page_cancel(self);
    g_clear_object(&self->api);
    g_clear_object(&self->root);
    self->status = NULL;
    self->title = NULL;
    self->date = NULL;
    self->description = NULL;
    self->picture = NULL;
    self->comments = NULL;

    G_OBJECT_CLASS(articles_detail_page_parent_class)->dispose(object);
}

static void articles_detail_page_class_init(ArticlesDetailPageClass *class) {
    GObjectClass *object_class = G_OBJECT_CLASS(class);
    object_class->dispose = articles_detail_page_dispose;
}

static void articles_detail_page_init(ArticlesDetailPage *self) { (void)self; }

ArticlesDetailPage *articles_detail_page_new(
    GtkBuilder *builder,
    ArticlesApi *api,
    ArticlesDetailPageBackCallback back_callback,
    gpointer user_data
) {
    g_return_val_if_fail(GTK_IS_BUILDER(builder), NULL);
    g_return_val_if_fail(ARTICLES_IS_API(api), NULL);
    g_return_val_if_fail(back_callback != NULL, NULL);

    GtkWidget *root =
        GTK_WIDGET(gtk_builder_get_object(builder, "article_detail_scroll"));
    GtkButton *back_button =
        GTK_BUTTON(gtk_builder_get_object(builder, "article_back_button"));
    GtkLabel *status =
        GTK_LABEL(gtk_builder_get_object(builder, "article_detail_status"));
    GtkLabel *title =
        GTK_LABEL(gtk_builder_get_object(builder, "article_detail_title"));
    GtkLabel *date =
        GTK_LABEL(gtk_builder_get_object(builder, "article_detail_date"));
    GtkLabel *description = GTK_LABEL(
        gtk_builder_get_object(builder, "article_detail_description")
    );
    GtkPicture *picture =
        GTK_PICTURE(gtk_builder_get_object(builder, "article_detail_picture"));
    GtkListBox *comments =
        GTK_LIST_BOX(gtk_builder_get_object(builder, "article_comments"));
    g_return_val_if_fail(GTK_IS_WIDGET(root), NULL);
    g_return_val_if_fail(GTK_IS_BUTTON(back_button), NULL);
    g_return_val_if_fail(GTK_IS_LABEL(status), NULL);
    g_return_val_if_fail(GTK_IS_LABEL(title), NULL);
    g_return_val_if_fail(GTK_IS_LABEL(date), NULL);
    g_return_val_if_fail(GTK_IS_LABEL(description), NULL);
    g_return_val_if_fail(GTK_IS_PICTURE(picture), NULL);
    g_return_val_if_fail(GTK_IS_LIST_BOX(comments), NULL);

    ArticlesDetailPage *self = g_object_new(ARTICLES_TYPE_DETAIL_PAGE, NULL);
    self->root = g_object_ref(root);
    self->status = status;
    self->title = title;
    self->date = date;
    self->description = description;
    self->picture = picture;
    self->comments = comments;
    self->api = g_object_ref(api);
    self->back_callback = back_callback;
    self->callback_data = user_data;
    g_signal_connect(back_button, "clicked", G_CALLBACK(on_back_clicked), self);

    return self;
}

void articles_detail_page_load(
    ArticlesDetailPage *self,
    const gchar *article_id
) {
    g_return_if_fail(ARTICLES_IS_DETAIL_PAGE(self));
    g_return_if_fail(article_id != NULL);

    if (self->closing) {
        return;
    }

    if (self->cancellable) {
        g_cancellable_cancel(self->cancellable);
    }
    g_clear_object(&self->cancellable);
    self->cancellable = g_cancellable_new();
    self->generation++;
    article_picture_cancel(self->picture);
    gtk_picture_set_paintable(self->picture, NULL);
    gtk_label_set_text(self->status, "Loading article…");
    articles_api_get_async(
        self->api,
        article_id,
        self->cancellable,
        article_detail_ready,
        articles_detail_page_request_new(self)
    );
}

void articles_detail_page_cancel(ArticlesDetailPage *self) {
    g_return_if_fail(ARTICLES_IS_DETAIL_PAGE(self));

    self->closing = TRUE;
    self->generation++;
    if (self->cancellable) {
        g_cancellable_cancel(self->cancellable);
    }
    g_clear_object(&self->cancellable);
    if (self->picture) {
        article_picture_cancel(self->picture);
    }
}
