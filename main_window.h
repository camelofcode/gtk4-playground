#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <gtk/gtk.h>

void main_window_register_callbacks(GtkBuilderCScope *scope);

void main_window_init(
    GtkBuilder *builder,
    GtkApplication *app
);

#endif