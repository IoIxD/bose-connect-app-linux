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

#define BLE_SCAN_TIMEOUT 10

pthread_mutex_t adapter_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t agent_mutex   = PTHREAD_MUTEX_INITIALIZER;

bool adapter_do_scan = false;

pthread_mutex_t array_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef void (*ble_discovered_device_t)(const char *addr, const char *name);

// We use a mutex to make the BLE connections synchronous
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

static void on_device_connect(gattlib_adapter_t *adapter, const char *dst,
                              gattlib_connection_t *connection, int error,
                              void *user_data) {
  gattlib_primary_service_t *services;
  gattlib_characteristic_t  *characteristics;
  int                        services_count, characteristics_count;
  char                       uuid_str[MAX_LEN_UUID_STR + 1];
  int                        ret, i;

  ret = gattlib_discover_primary(connection, &services, &services_count);
  if (ret != 0) {
    GATTLIB_LOG(GATTLIB_ERROR, "Fail to discover primary services.");
    goto disconnect_exit;
  }

  for (i = 0; i < services_count; i++) {
    gattlib_uuid_to_string(&services[i].uuid, uuid_str, sizeof(uuid_str));

    printf("service[%d] start_handle:%02x end_handle:%02x uuid:%s\n", i,
           services[i].attr_handle_start, services[i].attr_handle_end,
           uuid_str);
  }
  free(services);

  ret = gattlib_discover_char(connection, &characteristics,
                              &characteristics_count);
  if (ret != 0) {
    GATTLIB_LOG(GATTLIB_ERROR, "Fail to discover characteristics.");
    goto disconnect_exit;
  }
  for (i = 0; i < characteristics_count; i++) {
    gattlib_uuid_to_string(&characteristics[i].uuid, uuid_str,
                           sizeof(uuid_str));

    printf("characteristic[%d] properties:%02x value_handle:%04x uuid:%s\n", i,
           characteristics[i].properties, characteristics[i].value_handle,
           uuid_str);
  }
  free(characteristics);

disconnect_exit:
  gattlib_disconnect(connection, false /* wait_disconnection */);
}

static void *ble_connect_device(void *arg) {
  struct connection_t *connection = arg;
  char                *addr       = connection->address;
  int                  ret;

  pthread_mutex_lock(&g_mutex);
  printf("------------START %s ---------------\n", addr);

  ret =
      gattlib_connect(connection->adapter, connection->address,
                      GATTLIB_CONNECTION_OPTIONS_NONE, on_device_connect, NULL);
  if (ret != GATTLIB_SUCCESS) {
    GATTLIB_LOG(GATTLIB_ERROR, "Failed to connect to the bluetooth device '%s'",
                connection->address);
  }

  printf("------------DONE %s ---------------\n", addr);
  pthread_mutex_unlock(&g_mutex);
  return NULL;
}

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
  connection->adapter = adapter;

  ret =
      pthread_create(&connection->thread, NULL, ble_connect_device, connection);
  if (ret != 0) {
    GATTLIB_LOG(GATTLIB_ERROR, "Failt to create BLE connection thread.");
    free(connection);
    return;
  }
  LIST_INSERT_HEAD(&connections, connection, entries);
}

static void *ble_task(void *arg) {
  gattlib_adapter_t *adapter;
  int                ret;

  ret = gattlib_adapter_open("hci0", &adapter);
  if (ret) {
    GATTLIB_LOG(GATTLIB_ERROR, "Failed to open adapter.");
    return NULL;
  }

  pthread_mutex_lock(&g_mutex);
  ret = gattlib_adapter_scan_enable(adapter, ble_discovered_device,
                                    BLE_SCAN_TIMEOUT, NULL /* user_data */);
  if (ret) {
    GATTLIB_LOG(GATTLIB_ERROR, "Failed to scan.");
    goto EXIT;
  }

  gattlib_adapter_scan_disable(adapter);

  printf("Scan completed\n");
  pthread_mutex_unlock(&g_mutex);

  // // Wait for the thread to complete
  while (connections.lh_first != NULL) {
    struct connection_t *connection = connections.lh_first;
    pthread_join(connection->thread, NULL);
    // LIST_REMOVE(connections.lh_first, entries);
    // free(connection);
  }

EXIT:
  // gattlib_adapter_close(adapter);
  return NULL;
}

void *bluetooth_thread_start() {
  int ret;

#ifdef GATTLIB_LOG_BACKEND_SYSLOG
  openlog("gattlib_ble_scan", LOG_CONS | LOG_NDELAY | LOG_PERROR, LOG_USER);
  setlogmask(LOG_UPTO(LOG_INFO));
#endif

  LIST_INIT(&connections);

  ret = gattlib_mainloop(ble_task, NULL);
  if (ret != GATTLIB_SUCCESS) {
    GATTLIB_LOG(GATTLIB_ERROR, "Failed to create gattlib mainloop");
  }

  return NULL;
}

void bluetooth_get_names(struct DeviceInfo devices[255], size_t *len,
                         size_t max_len) {
  pthread_mutex_lock(&array_mutex);

  int n = 0;
  while (connections.lh_first != NULL) {
    struct connection_t *connection = connections.lh_first;
    strncpy(devices[n].address, connection->address, 255);
    strncpy(devices[n].name, connection->name, 255);

    // LIST_REMOVE(connections.lh_first, entries);
    // free(connection);
    n++;
    if (n >= max_len) {
      break;
    }
  }

  pthread_mutex_unlock(&array_mutex);
}