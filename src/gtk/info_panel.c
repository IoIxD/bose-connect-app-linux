#include "../library/based.h"
#include "panels.h"
#include <glib.h>
#include <gtk/gtk.h>
#include <gtk/gtkshortcut.h>
#include <stdio.h>
#include <unistd.h>

struct DeviceInfo *currentDevice;
GtkIconTheme      *icon_theme;
GtkIconPaintable  *battery_icon;

int headphone_socket;

struct {
  unsigned int bat_level;
} device_stats;

void headphone_socket_open() {
  if (headphone_socket != -1) {
    close(headphone_socket);
  }

  headphone_socket = socket_init(currentDevice->address);
}

void info_push_device_name(GtkWidget *info_box) {
  // Device Name
  gtk_box_append(GTK_BOX(info_box), gtk_label_new(currentDevice->name));
  gtk_box_append(GTK_BOX(info_box),
                 gtk_separator_new(GTK_ORIENTATION_VERTICAL));

  gtk_window_set_child(GTK_WINDOW(window), info_box);
}

void info_push_battery_info(GtkWidget *info_box) {
  // Battery
  char bat[255];
  char bat_icon[255];

  unsigned int status   = 0;
  int          attempts = 0;
  // If the battery command ignores 0, ignore it and try again.
  while (status <= 5) {
    status = get_battery_level(headphone_socket, &device_stats.bat_level);

    // but we don't wanna fry it so we do it intervals and give up eventually.
    usleep(100);
    attempts++;
    if (attempts >= 50) {
      device_stats.bat_level = 0;
      break;
    }
  }
  device_stats.bat_level = status;

  // The regular battery level
  snprintf(bat, 255, "%d", device_stats.bat_level);

  // The GTK icon that represents this battery level
  snprintf(bat_icon, 255, "battery-%03d",
           (int)(floor((float)device_stats.bat_level / 10) * 10));

  battery_icon = gtk_icon_theme_lookup_icon(icon_theme,
                                            bat_icon, // icon name
                                            NULL,
                                            24, // icon size
                                            1,  // scale
                                            GTK_TEXT_DIR_LTR, 0);

  GtkWidget *battery_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

  GtkWidget *battery_text = gtk_label_new(bat);
  gtk_box_append(GTK_BOX(battery_box),
                 gtk_picture_new_for_paintable(GDK_PAINTABLE(battery_icon)));
  gtk_box_append(GTK_BOX(battery_box), battery_text);

  gtk_box_append(GTK_BOX(info_box), battery_box);
}

int info_update() {
  pthread_mutex_lock(&gtk_mutex);

  headphone_socket_open();

  GtkWidget *info_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_widget_set_margin_top(info_box, 50);
  gtk_widget_set_margin_bottom(info_box, 50);
  gtk_widget_set_margin_start(info_box, 50);
  gtk_widget_set_margin_end(info_box, 50);

  info_push_device_name(info_box);
  info_push_battery_info(info_box);

  pthread_mutex_unlock(&gtk_mutex);

  return 1;
}

void set_info_panel(GtkWidget *app, gpointer user_data) {
  icon_theme = gtk_icon_theme_get_for_display(gtk_widget_get_display(app));

  info_update();
  g_timeout_add(10 * 1000, info_update, NULL);
}
