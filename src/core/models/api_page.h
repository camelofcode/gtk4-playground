#ifndef API_PAGE_H
#define API_PAGE_H

#include <gio/gio.h>

G_BEGIN_DECLS

#define API_TYPE_PAGE (api_page_get_type())
G_DECLARE_FINAL_TYPE(
    ApiPage,
    api_page,
    API,
    PAGE,
    GObject
)

ApiPage *api_page_new(
    guint current_page,
    guint total_pages,
    GListModel *items
);

/* Deserializes each array item as the supplied concrete GObject type. */
ApiPage *api_page_from_json(
    const gchar *json,
    gssize length,
    GType item_type,
    GError **error
);

guint api_page_get_current_page(ApiPage *self);
guint api_page_get_total_pages(ApiPage *self);
GListModel *api_page_get_items(ApiPage *self);

G_END_DECLS

#endif
