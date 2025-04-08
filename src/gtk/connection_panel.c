#include "bluetooth.h"
#include "panels.h"
#include <gtk/gtk.h>

void lmao(GtkWidget *app, gpointer user_data) { printf("%s\n", user_data); }

void set_connection_panel(GtkWidget *app, gpointer user_data) {
  pthread_mutex_lock(&gtk_mutex);

  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_widget_set_margin_top(vbox, 50);
  gtk_widget_set_margin_bottom(vbox, 50);
  gtk_widget_set_margin_start(vbox, 50);
  gtk_widget_set_margin_end(vbox, 50);

  gtk_box_append(GTK_BOX(vbox), gtk_label_new("Choose BOSE Headset"));

  bluetooth_get_names(bluetooth_names, bluetooth_addresses, &bluetooth_arr_len,
                      255);

  for (int i = 0; i < bluetooth_arr_len; i++) {
    GtkWidget *button = gtk_button_new_with_label(bluetooth_names[i]);
    gtk_box_append(GTK_BOX(vbox), button);
    g_signal_connect(button, "clicked", G_CALLBACK(lmao),
                     &bluetooth_addresses[i]);
  }

  gtk_box_append(GTK_BOX(vbox), gtk_separator_new(GTK_ORIENTATION_VERTICAL));

  reloadButton = gtk_button_new_with_label("Reload");
  gtk_box_append(GTK_BOX(vbox), reloadButton);

  g_signal_connect(reloadButton, "clicked", G_CALLBACK(set_connection_panel),
                   NULL);

  gtk_window_set_child(GTK_WINDOW(window), vbox);

  pthread_mutex_unlock(&gtk_mutex);
}

void init_connection_panel() { pthread_mutex_init(&gtk_mutex, NULL); }