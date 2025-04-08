
#include <gtk/gtk.h>
#include <pthread.h>

#ifndef __PANELS_H
#define __PANELS_H

extern pthread_t bluetooth_thread;

extern GtkWidget      *window;
extern GtkWidget      *reloadButton;
extern pthread_mutex_t gtk_mutex;

extern char   bluetooth_names[255][255];
extern char   bluetooth_addresses[255][255];
extern size_t bluetooth_arr_len;

int  panel_loop();
void set_connection_panel(GtkWidget *app, gpointer user_data);

#endif