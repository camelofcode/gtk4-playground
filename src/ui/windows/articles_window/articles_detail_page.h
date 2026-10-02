#ifndef ARTICLES_DETAIL_PAGE_H
#define ARTICLES_DETAIL_PAGE_H

#include "src/core/articles_api.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ARTICLES_TYPE_DETAIL_PAGE (articles_detail_page_get_type())
G_DECLARE_FINAL_TYPE(
    ArticlesDetailPage,
    articles_detail_page,
    ARTICLES,
    DETAIL_PAGE,
    GObject
)

typedef void (*ArticlesDetailPageBackCallback)(
    ArticlesDetailPage *self,
    gpointer user_data
);

ArticlesDetailPage *articles_detail_page_new(
    GtkBuilder *builder,
    ArticlesApi *api,
    ArticlesDetailPageBackCallback back_callback,
    gpointer user_data
);

void articles_detail_page_load(
    ArticlesDetailPage *self,
    const gchar *article_id
);

void articles_detail_page_cancel(ArticlesDetailPage *self);

G_END_DECLS

#endif
