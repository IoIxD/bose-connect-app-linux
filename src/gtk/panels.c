#include "panels.h"
#include "bluetooth.h"

pthread_t bluetooth_thread;

GtkWidget      *window;
GtkWidget      *reloadButton;
pthread_mutex_t gtk_mutex = PTHREAD_MUTEX_INITIALIZER;

struct DeviceInfo devices[255];
size_t            bluetooth_arr_len = 0;

void activate(GtkApplication *app, gpointer user_data) {
  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "BOSE Headphone Manager");
  gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);

  set_connection_panel(NULL, 0);

  gtk_window_present(GTK_WINDOW(window));
}

int panel_loop() {
  pthread_create(&bluetooth_thread, NULL, bluetooth_thread_start, NULL);

  GtkApplication *app;
  int             status;

  app = gtk_application_new("net.ioi-xd.bose-gtk", G_APPLICATION_DEFAULT_FLAGS);

  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), 0, NULL);
  g_object_unref(app);

  return status;
}