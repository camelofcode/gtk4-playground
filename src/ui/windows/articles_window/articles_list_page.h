#ifndef ARTICLES_LIST_PAGE_H
#define ARTICLES_LIST_PAGE_H

#include "src/core/articles_api.h"
#include "src/core/models/article_summary.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ARTICLES_TYPE_LIST_PAGE (articles_list_page_get_type())
G_DECLARE_FINAL_TYPE(
    ArticlesListPage,
    articles_list_page,
    ARTICLES,
    LIST_PAGE,
    GObject
)

typedef void (*ArticlesListPageOpenCallback)(
    ArticlesListPage *self,
    ArticleSummary *summary,
    gpointer user_data
);

ArticlesListPage *articles_list_page_new(
    GtkBuilder *builder,
    ArticlesApi *api,
    ArticlesListPageOpenCallback open_callback,
    gpointer user_data
);

void articles_list_page_load_initial(ArticlesListPage *self);
void articles_list_page_cancel(ArticlesListPage *self);

G_END_DECLS

#endif
