#ifndef GOBJECT_HELPERS_H
#define GOBJECT_HELPERS_H

#include <gio/gio.h>

GParamSpec *gobject_helpers_string_property(const gchar *name);
GParamSpec *gobject_helpers_list_store_property(const gchar *name);

#endif
