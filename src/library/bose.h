#ifndef BOSE_CONNECT_APP_LINUX_SRC_MAIN_H
#define BOSE_CONNECT_APP_LINUX_SRC_MAIN_H

int get_socket(char *address);

void usage();
int  do_get_information(char *address);
int  do_set_name(char *address, const char *arg);
int  do_set_prompt_language(char *address, const char *arg);
int  do_set_voice_prompts(char *address, const char *arg);
int  do_set_auto_off(char *address, const char *arg);
int  do_set_noise_cancelling(char *address, const char *arg);
int  do_get_device_status(char *address);
int  do_set_pairing(char *address, const char *arg);
int  do_set_self_voice(char *address, const char *arg);
int  do_get_firmware_version(char *address);
int  do_get_serial_number(char *address);
int  do_get_battery_level(char *address);
int  do_get_paired_devices(char *address);
int  do_connect_device(char *address, const char *arg);
int  do_disconnect_device(char *address, const char *arg);
int  do_remove_device(char *address, const char *arg);
int  do_get_device_id(char *address);
int  do_send_packet(char *address, const char *arg);

#define OPTION_DEVICE_ID         5
#define OPTION_CONNECT_DEVICE    2
#define OPTION_DISCONNECT_DEVICE 3
#define OPTION_REMOVE_DEVICE     4
#define OPTION_SEND_PACKET       1

#endif
