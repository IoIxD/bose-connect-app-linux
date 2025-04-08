#ifndef __GTK_BLUETOOTH_H
#define __GTK_BLUETOOTH_H

#include <gtk/gtk.h>

void *bluetooth_thread_start();
void  bluetooth_get_names(char names[255][255], char addresses[255][255],
                          size_t *len, size_t max_len);

#endif