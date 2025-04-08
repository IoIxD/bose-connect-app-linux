#include "bluetooth.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

#ifdef GATTLIB_LOG_BACKEND_SYSLOG
#include <syslog.h>
#endif

#include "gattlib.h"

struct connection_t {
  pthread_t          thread;
  gattlib_adapter_t *adapter;
  LIST_ENTRY(connection_t) entries;
  char name[255];
  char address[255];
};

LIST_HEAD(listhead, connection_t) connections;

bool adapter_do_scan = false;

pthread_mutex_t tired_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef void (*ble_discovered_device_t)(const char *addr, const char *name);

// We use a mutex to make the BLE connections synchronous
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

static void ble_discovered_device(gattlib_adapter_t *adapter, const char *addr,
                                  const char *name, void *user_data) {
  struct connection_t *connection;
  int                  ret;

  connection = calloc(sizeof(struct connection_t), 1);
  if (connection == NULL) {
    GATTLIB_LOG(GATTLIB_ERROR, "Failt to allocate connection.");
    return;
  }

  strncpy(connection->address, addr, 255);

  if (name == NULL) {
    strncpy(connection->name, addr, 255);
  } else {
    strncpy(connection->name, name, 255);
  }

  connection->adapter = adapter;

  LIST_INSERT_HEAD(&connections, connection, entries);
}
bool did_bt = false;

static void *ble_task(void *arg) {
  gattlib_adapter_t *adapter;
  int                ret;

  ret = gattlib_adapter_open(NULL, &adapter);
  if (ret) {
    GATTLIB_LOG(GATTLIB_ERROR, "Failed to open adapter.");
    return NULL;
  }

  pthread_mutex_lock(&g_mutex);
  ret = gattlib_adapter_scan_enable(adapter, ble_discovered_device, 1,
                                    NULL /* user_data */);
  if (ret) {
    GATTLIB_LOG(GATTLIB_ERROR, "Failed to scan.");
    goto EXIT;
  }

  gattlib_adapter_scan_disable(adapter);

  pthread_mutex_unlock(&g_mutex);

EXIT:
  gattlib_adapter_close(adapter);
  return NULL;
}
void *bluetooth_thread_thing(void *) {

  int ret;

  LIST_INIT(&connections);

  ret = gattlib_mainloop(ble_task, devices);
  if (ret != GATTLIB_SUCCESS) {
    GATTLIB_LOG(GATTLIB_ERROR, "Failed to create gattlib mainloop");
  }

  return NULL;
}

void bluetooth_get_names(struct DeviceInfo devices[255], size_t *len,
                         size_t max_len) {
  pthread_mutex_lock(&tired_mutex);
  bluetooth_thread_thing(NULL);

  int n = 0;

  struct connection_t *connection = LIST_FIRST(&connections);

  while (connection != NULL) {
    printf("%s, %s\n", connection->address, connection->name);

    strncpy(devices[n].address, connection->address, 255);
    strncpy(devices[n].name, connection->name, 255);

    n++;
    if (n >= 255) {
      break;
    }
    connection = LIST_NEXT(connection, entries);
  }

  *len = n;

  pthread_mutex_unlock(&tired_mutex);
}