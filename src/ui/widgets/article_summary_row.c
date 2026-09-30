#include "article_summary_row.h"

#include "article_picture.h"
#include "src/core/helpers/date_helpers.h"

enum { OPEN_REQUESTED, N_SIGNALS };

struct _ArticleSummaryRow {
    GtkListBoxRow parent_instance;
    GtkPicture *picture;
    GtkLabel *title;
    GtkLabel *date;
    GtkButton *open_button;
    ArticleSummary *summary;
};

static guint signals[N_SIGNALS];

G_DEFINE_TYPE(
    ArticleSummaryRow,
    article_summary_row,
    GTK_TYPE_LIST_BOX_ROW
)

static void on_open_clicked(
    GtkButton *button,
    ArticleSummaryRow *self
) {
    (void)button;
    g_signal_emit(self, signals[OPEN_REQUESTED], 0, self->summary);
}

static void article_summary_row_dispose(GObject *object) {
    ArticleSummaryRow *self = ARTICLE_SUMMARY_ROW(object);
    g_clear_object(&self->summary);
    gtk_widget_dispose_template(GTK_WIDGET(self), ARTICLE_TYPE_SUMMARY_ROW);
    G_OBJECT_CLASS(article_summary_row_parent_class)->dispose(object);
}

static void article_summary_row_class_init(ArticleSummaryRowClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    object_class->dispose = article_summary_row_dispose;

    signals[OPEN_REQUESTED] = g_signal_new(
        "open-requested",
        G_TYPE_FROM_CLASS(klass),
        G_SIGNAL_RUN_LAST,
        0,
        NULL,
        NULL,
        NULL,
        G_TYPE_NONE,
        1,
        ARTICLE_TYPE_SUMMARY
    );

    gtk_widget_class_set_template_from_resource(
        widget_class,
        "/com/camelofdoom/gtk4-playground/ui/widgets/article-summary-row.ui"
    );
    gtk_widget_class_bind_template_child(
        widget_class,
        ArticleSummaryRow,
        picture
    );
    gtk_widget_class_bind_template_child(
        widget_class,
        ArticleSummaryRow,
        title
    );
    gtk_widget_class_bind_template_child(widget_class, ArticleSummaryRow, date);
    gtk_widget_class_bind_template_child(
        widget_class,
        ArticleSummaryRow,
        open_button
    );
}

static void article_summary_row_init(ArticleSummaryRow *self) {
    gtk_widget_init_template(GTK_WIDGET(self));
    g_signal_connect(
        self->open_button,
        "clicked",
        G_CALLBACK(on_open_clicked),
        self
    );
}

ArticleSummaryRow *article_summary_row_new(
    ArticleSummary *summary,
    ArticlesApi *api
) {
    g_return_val_if_fail(ARTICLE_IS_SUMMARY(summary), NULL);
    g_return_val_if_fail(ARTICLES_IS_API(api), NULL);

    ArticleSummaryRow *self = g_object_new(ARTICLE_TYPE_SUMMARY_ROW, NULL);
    self->summary = g_object_ref(summary);
    gtk_label_set_text(self->title, article_summary_get_name(summary));
    g_autofree gchar *date =
        date_helpers_format_iso8601(article_summary_get_date(summary));
    gtk_label_set_text(self->date, date);
    article_picture_load(
        self->picture,
        api,
        article_summary_get_image(summary)
    );
    return self;
}

ArticleSummary *article_summary_row_get_summary(ArticleSummaryRow *self) {
    g_return_val_if_fail(ARTICLE_IS_SUMMARY_ROW(self), NULL);
    return self->summary;
}
