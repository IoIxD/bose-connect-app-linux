#ifndef BOSE_CONNECT_APP_LINUX_SRC_LIBRARY_UTIL_H
#define BOSE_CONNECT_APP_LINUX_SRC_LIBRARY_UTIL_H

#include <bluetooth/bluetooth.h>
#include <stdint.h>

#define MAX_HEXADECIMAL_UNIT 16
#define MAX_DECIMAL_UNIT     10
#define ASCII_NUMBER_ZERO    48
#define ASCII_LETTER_A       65

int str_to_byte(const char *str, uint8_t *byte);

void unit_to_hex_string(int number, char *dest);

#endif
