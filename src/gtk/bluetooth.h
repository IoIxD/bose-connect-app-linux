#ifndef __GTK_BLUETOOTH_H
#define __GTK_BLUETOOTH_H

#include "gtk.h"
#include <gtk/gtk.h>

void bluetooth_get_names(struct DeviceInfo devices[255], size_t *len,
                         size_t max_len);

#endif