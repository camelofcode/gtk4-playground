#include "article_comment_row.h"

#include "src/core/helpers/date_helpers.h"

struct _ArticleCommentRow {
    GtkListBoxRow parent_instance;
    GtkLabel *user;
    GtkLabel *date;
    GtkLabel *text;
};

G_DEFINE_TYPE(
    ArticleCommentRow,
    article_comment_row,
    GTK_TYPE_LIST_BOX_ROW
)

static void article_comment_row_dispose(GObject *object) {
    gtk_widget_dispose_template(GTK_WIDGET(object), ARTICLE_TYPE_COMMENT_ROW);
    G_OBJECT_CLASS(article_comment_row_parent_class)->dispose(object);
}

static void article_comment_row_class_init(ArticleCommentRowClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    object_class->dispose = article_comment_row_dispose;
    gtk_widget_class_set_template_from_resource(
        widget_class,
        "/com/camelofdoom/gtk4-playground/ui/widgets/article-comment-row.ui"
    );
    gtk_widget_class_bind_template_child(widget_class, ArticleCommentRow, user);
    gtk_widget_class_bind_template_child(widget_class, ArticleCommentRow, date);
    gtk_widget_class_bind_template_child(widget_class, ArticleCommentRow, text);
}

static void article_comment_row_init(ArticleCommentRow *self) {
    gtk_widget_init_template(GTK_WIDGET(self));
}

ArticleCommentRow *article_comment_row_new(ArticleComment *comment) {
    g_return_val_if_fail(ARTICLE_IS_COMMENT(comment), NULL);

    ArticleCommentRow *self = g_object_new(ARTICLE_TYPE_COMMENT_ROW, NULL);
    gtk_label_set_text(self->user, article_comment_get_user(comment));
    g_autofree gchar *date =
        date_helpers_format_iso8601(article_comment_get_date(comment));
    gtk_label_set_text(self->date, date);
    gtk_label_set_text(self->text, article_comment_get_text(comment));
    return self;
}
