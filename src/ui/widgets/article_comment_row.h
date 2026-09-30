#ifndef ARTICLE_COMMENT_ROW_H
#define ARTICLE_COMMENT_ROW_H

#include "src/core/models/article_comment.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ARTICLE_TYPE_COMMENT_ROW (article_comment_row_get_type())
G_DECLARE_FINAL_TYPE(
    ArticleCommentRow,
    article_comment_row,
    ARTICLE,
    COMMENT_ROW,
    GtkListBoxRow
)

ArticleCommentRow *article_comment_row_new(ArticleComment *comment);

G_END_DECLS

#endif
