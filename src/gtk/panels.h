
#include <gtk/gtk.h>
#include <pthread.h>

#ifndef __PANELS_H
#define __PANELS_H

extern pthread_t bluetooth_thread;

extern GtkWidget      *window;
extern GtkWidget      *reloadButton;
extern pthread_mutex_t gtk_mutex;

extern int headphone_socket;

struct DeviceInfo {
  char name[255];
  char address[255];
};

extern struct DeviceInfo  devices[255];
extern struct DeviceInfo *currentDevice;

extern size_t bluetooth_arr_len;

int  panel_loop();
void set_connection_panel(GtkWidget *app, gpointer user_data);

void set_info_panel(GtkWidget *app, gpointer user_data);

#endif