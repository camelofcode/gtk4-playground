#ifndef ARTICLES_API_H
#define ARTICLES_API_H

#include "models/api_page.h"
#include "models/article_detail.h"
#include <gdk/gdk.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define ARTICLES_TYPE_API (articles_api_get_type())
G_DECLARE_FINAL_TYPE(
    ArticlesApi,
    articles_api,
    ARTICLES,
    API,
    GObject
)

ArticlesApi *articles_api_new(const gchar *base_url);
void articles_api_list_async(
    ArticlesApi *self,
    guint page,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data
);
ApiPage *articles_api_list_finish(
    ArticlesApi *self,
    GAsyncResult *result,
    GError **error
);
void articles_api_get_async(
    ArticlesApi *self,
    const gchar *article_id,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data
);
ArticleDetail *articles_api_get_finish(
    ArticlesApi *self,
    GAsyncResult *result,
    GError **error
);
void articles_api_download_image_async(
    ArticlesApi *self,
    const gchar *url,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data
);
GdkTexture *articles_api_download_image_finish(
    ArticlesApi *self,
    GAsyncResult *result,
    GError **error
);

G_END_DECLS
#endif
