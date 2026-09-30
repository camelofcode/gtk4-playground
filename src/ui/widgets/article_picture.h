#ifndef ARTICLE_PICTURE_H
#define ARTICLE_PICTURE_H

#include "src/core/articles_api.h"
#include <gtk/gtk.h>

void article_picture_load(
    GtkPicture *picture,
    ArticlesApi *api,
    const gchar *url
);
void article_picture_cancel(GtkPicture *picture);

#endif
