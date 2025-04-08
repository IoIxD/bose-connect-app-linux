#include "bluetooth.h"
#include "gtk.h"
#include <gtk/gtk.h>
#include <pthread.h>
#include <string.h>

void device_select(GtkWidget *app, gpointer user_data) {
  struct DeviceInfo *device = (struct DeviceInfo *)user_data;

  currentDevice = device;

  printf("name %s\n", device->name);
  printf("address %s\n", device->address);

  set_info_panel(app, user_data);
}

GtkWidget *vbox;

void set_connection_panel(GtkWidget *app, gpointer user_data) {
  pthread_mutex_lock(&gtk_mutex);

  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_widget_set_margin_top(vbox, 50);
  gtk_widget_set_margin_bottom(vbox, 50);
  gtk_widget_set_margin_start(vbox, 50);
  gtk_widget_set_margin_end(vbox, 50);

  gtk_box_append(GTK_BOX(vbox), gtk_label_new("Choose BOSE Headset"));

  bluetooth_get_names(devices, &bluetooth_arr_len, 255);

  for (int i = 0; i < bluetooth_arr_len; i++) {
    GtkWidget *button = gtk_button_new_with_label(devices[i].name);
    gtk_box_append(GTK_BOX(vbox), button);
    g_signal_connect(button, "clicked", G_CALLBACK(device_select), &devices[i]);
  }

  gtk_box_append(GTK_BOX(vbox), gtk_separator_new(GTK_ORIENTATION_VERTICAL));

  gtk_window_set_child(GTK_WINDOW(window), vbox);

  pthread_mutex_unlock(&gtk_mutex);
}

void init_connection_panel() {
  pthread_mutex_init(&gtk_mutex, NULL);
  pthread_mutex_init(&tired_mutex, NULL);
}