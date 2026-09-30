#ifndef ARTICLE_SUMMARY_H
#define ARTICLE_SUMMARY_H

#include <glib-object.h>

G_BEGIN_DECLS

#define ARTICLE_TYPE_SUMMARY (article_summary_get_type())
G_DECLARE_FINAL_TYPE(
    ArticleSummary,
    article_summary,
    ARTICLE,
    SUMMARY,
    GObject
)

ArticleSummary *article_summary_new(
    const gchar *id,
    const gchar *name,
    const gchar *image,
    const gchar *date
);

const gchar *article_summary_get_id(ArticleSummary *self);
const gchar *article_summary_get_name(ArticleSummary *self);
const gchar *article_summary_get_image(ArticleSummary *self);
const gchar *article_summary_get_date(ArticleSummary *self);

G_END_DECLS

#endif
