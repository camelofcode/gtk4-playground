#ifndef ARTICLE_COMMENT_H
#define ARTICLE_COMMENT_H

#include <glib-object.h>

G_BEGIN_DECLS

#define ARTICLE_TYPE_COMMENT (article_comment_get_type())
G_DECLARE_FINAL_TYPE(
    ArticleComment,
    article_comment,
    ARTICLE,
    COMMENT,
    GObject
)

ArticleComment *article_comment_new(
    const gchar *user,
    const gchar *date,
    const gchar *text
);

const gchar *article_comment_get_user(ArticleComment *self);
const gchar *article_comment_get_date(ArticleComment *self);
const gchar *article_comment_get_text(ArticleComment *self);

G_END_DECLS

#endif
