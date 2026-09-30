#include "date_helpers.h"

gchar *date_helpers_format_iso8601(const gchar *iso_date) {
    if (!iso_date || !*iso_date) {
        return g_strdup("");
    }

    g_autoptr(GDateTime) date = g_date_time_new_from_iso8601(iso_date, NULL);
    return date ? g_date_time_format(date, "%e %B %Y") : g_strdup(iso_date);
}
