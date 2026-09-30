#ifndef ARTICLE_DETAIL_H
#define ARTICLE_DETAIL_H

#include <gio/gio.h>

#include "article_comment.h"

G_BEGIN_DECLS

#define ARTICLE_TYPE_DETAIL (article_detail_get_type())
G_DECLARE_FINAL_TYPE(
    ArticleDetail,
    article_detail,
    ARTICLE,
    DETAIL,
    GObject
)

ArticleDetail *article_detail_new(
    const gchar *id,
    const gchar *name,
    const gchar *image,
    const gchar *date,
    const gchar *description
);

const gchar *article_detail_get_id(ArticleDetail *self);
const gchar *article_detail_get_name(ArticleDetail *self);
const gchar *article_detail_get_image(ArticleDetail *self);
const gchar *article_detail_get_date(ArticleDetail *self);
const gchar *article_detail_get_description(ArticleDetail *self);
GListModel *article_detail_get_comments(ArticleDetail *self);

G_END_DECLS

#endif
