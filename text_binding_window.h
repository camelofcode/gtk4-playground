#ifndef TEXT_BINDING_WINDOW_H
#define TEXT_BINDING_WINDOW_H

#include <gtk/gtk.h>

void text_binding_window_register_callbacks(
    GtkBuilderCScope *scope
);

void text_binding_window_init(
    GtkBuilder *builder,
    GtkApplication *app
);

#endif