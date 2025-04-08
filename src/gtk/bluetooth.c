#include "binc/adapter.h"
#include "binc/agent.h"
#include "binc/device.h"
#include "binc/forward_decl.h"
#include "binc/logger.h"
#include "panels.h"
#include <bits/pthreadtypes.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

GMainLoop *loop            = NULL;
Adapter   *default_adapter = NULL;
Agent     *agent           = NULL;

pthread_mutex_t adapter_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t agent_mutex   = PTHREAD_MUTEX_INITIALIZER;

bool adapter_do_scan = FALSE;

pthread_mutex_t array_mutex  = PTHREAD_MUTEX_INITIALIZER;
GPtrArray      *device_array = NULL;

pthread_mutex_t  dbus_mutex = PTHREAD_MUTEX_INITIALIZER;
GDBusConnection *dbusConnection;

gboolean on_request_authorization(Device *device) {
  log_info("Main", "requesting authorization for '%s",
           binc_device_get_name(device));
  return TRUE;
}

guint32 on_request_passkey(Device *device) {
  guint32 pass = 000000;
  log_info("Main", "requesting passkey for '%s", binc_device_get_name(device));
  log_info("Main", "Enter 6 digit pin code: ");
  int result = fscanf(stdin, "%d", &pass);
  if (result != 1) {
    log_info("Main", "didn't read a pin code");
  }
  return pass;
}

void on_powered_state_changed(Adapter *adapter, gboolean state) {
  pthread_mutex_lock(&adapter_mutex);

  if (state) {
    adapter_do_scan = true;
  }

  pthread_mutex_unlock(&adapter_mutex);
}

gboolean callback(gpointer data) {
  pthread_mutex_lock(&agent_mutex);
  pthread_mutex_lock(&adapter_mutex);

  if (agent != NULL) {
    binc_agent_free(agent);
    agent = NULL;
  }

  if (default_adapter != NULL) {
    binc_adapter_free(default_adapter);
    default_adapter = NULL;
  }

  g_main_loop_quit((GMainLoop *)data);

  pthread_mutex_unlock(&agent_mutex);
  pthread_mutex_unlock(&adapter_mutex);
  return FALSE;
}

void *bluetooth_thread_start() {
  pthread_mutex_init(&adapter_mutex, NULL);
  pthread_mutex_init(&agent_mutex, NULL);
  pthread_mutex_init(&array_mutex, NULL);
  pthread_mutex_init(&dbus_mutex, NULL);

  log_enabled(TRUE);
  log_set_level(LOG_INFO);

  // Get a DBus connection
  dbusConnection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);

  // Setup mainloop
  loop = g_main_loop_new(NULL, FALSE);

  // Get the default adapter
  default_adapter = binc_adapter_get_default(dbusConnection);

  if (default_adapter != NULL) {

    // Register an agent and set callbacks
    agent = binc_agent_create(default_adapter, "/org/bluez/BincAgent",
                              KEYBOARD_DISPLAY);
    binc_agent_set_request_authorization_cb(agent, &on_request_authorization);
    binc_agent_set_request_passkey_cb(agent, &on_request_passkey);

    // Make sure the adapter is on before starting scanning
    binc_adapter_set_powered_state_cb(default_adapter,
                                      &on_powered_state_changed);
    if (!binc_adapter_get_powered_state(default_adapter)) {
      binc_adapter_power_on(default_adapter);
    } else {
      adapter_do_scan = true;
    }
  } else {
    log_error("MAIN", "No default_adapter found");
  }

  while (!adapter_do_scan) {
  }

  for (;;) {
  }

  // Disconnect from DBus
  g_dbus_connection_close_sync(dbusConnection, NULL, NULL);
  g_object_unref(dbusConnection);

  return NULL;
}

void bluetooth_get_names(struct DeviceInfo devices[255], size_t *len,
                         size_t max_len) {
  pthread_mutex_lock(&array_mutex);
  pthread_mutex_lock(&dbus_mutex);

  device_array = binc_adapter_find_all(dbusConnection);

  if (device_array != NULL) {
    int n = 0;

    *len = 0;

    for (int i = 0; i < device_array->len && i < max_len; i++) {
      Adapter *adapter = (Adapter *)g_ptr_array_index(device_array, i);

      GList *list = binc_adapter_get_connected_devices(adapter);
      while (list != NULL) {
        Device *dev = (Device *)list->data;

        strncpy(devices[n].name, binc_device_get_name(dev), 255);
        strncpy(devices[n].address, binc_device_get_address(dev), 255);

        list = list->next;

        n++;
        *len = n;
      }
    }
  }

  pthread_mutex_unlock(&dbus_mutex);
  pthread_mutex_unlock(&array_mutex);
}