#ifndef DATE_HELPERS_H
#define DATE_HELPERS_H

#include <glib.h>

/* Returns an owned display date, the original text when parsing fails, or an
 * empty string when iso_date is NULL or empty.
 */
gchar *date_helpers_format_iso8601(const gchar *iso_date);

#endif
