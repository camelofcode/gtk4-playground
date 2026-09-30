#ifndef ARTICLE_SUMMARY_ROW_H
#define ARTICLE_SUMMARY_ROW_H

#include "src/core/articles_api.h"
#include "src/core/models/article_summary.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ARTICLE_TYPE_SUMMARY_ROW (article_summary_row_get_type())
G_DECLARE_FINAL_TYPE(
    ArticleSummaryRow,
    article_summary_row,
    ARTICLE,
    SUMMARY_ROW,
    GtkListBoxRow
)

ArticleSummaryRow *article_summary_row_new(
    ArticleSummary *summary,
    ArticlesApi *api
);
ArticleSummary *article_summary_row_get_summary(ArticleSummaryRow *self);

G_END_DECLS

#endif
