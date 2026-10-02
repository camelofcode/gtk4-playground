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

/* response and error are borrowed for the duration of the callback.
 * Exactly one of them is non-NULL.
 */
typedef void (*ArticlesApiListCallback)(
    ArticlesApi *self,
    ApiPage *response,
    const GError *error,
    gpointer user_data
);

typedef void (*ArticlesApiDetailCallback)(
    ArticlesApi *self,
    ArticleDetail *response,
    const GError *error,
    gpointer user_data
);

typedef void (*ArticlesApiImageCallback)(
    ArticlesApi *self,
    GdkTexture *response,
    const GError *error,
    gpointer user_data
);

ArticlesApi *articles_api_new(const gchar *base_url);

void articles_api_list_async(
    ArticlesApi *self,
    guint page,
    GCancellable *cancellable,
    ArticlesApiListCallback callback,
    gpointer user_data
);

void articles_api_get_async(
    ArticlesApi *self,
    const gchar *article_id,
    GCancellable *cancellable,
    ArticlesApiDetailCallback callback,
    gpointer user_data
);

void articles_api_download_image_async(
    ArticlesApi *self,
    const gchar *url,
    GCancellable *cancellable,
    ArticlesApiImageCallback callback,
    gpointer user_data
);

G_END_DECLS
#endif
