#if WITH_GUI
#include "gtk/gtk.h"

int main() { return panel_loop(); }
#else
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <getopt.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "library/cli.h"

int main(int argc, char *argv[]) {
  static const char         *short_opt  = "hidfsban:n:o:c:l:v:p:e:";
  static const struct option long_opt[] = {
      {"help", no_argument, NULL, 'h'},
      {"info", no_argument, NULL, 'i'},
      {"device-status", no_argument, NULL, 'd'},
      {"firmware-version", no_argument, NULL, 'f'},
      {"serial-number", no_argument, NULL, 's'},
      {"battery-level", no_argument, NULL, 'b'},
      {"paired-devices", no_argument, NULL, 'a'},
      {"device-id", no_argument, NULL, 5},
      {"name", required_argument, NULL, 'n'},
      {"auto-off", required_argument, NULL, 'o'},
      {"noise-cancelling", required_argument, NULL, 'c'},
      {"prompt-language", required_argument, NULL, 'l'},
      {"voice-prompts", required_argument, NULL, 'v'},
      {"pairing", required_argument, NULL, 'p'},
      {"self-voice", required_argument, NULL, 'e'},
      {"connect-device", required_argument, NULL, 2},
      {"disconnect-device", required_argument, NULL, 3},
      {"remove-device", required_argument, NULL, 4},
      {"send-packet", required_argument, NULL, 1},
      {0, no_argument, NULL, 0}};

  // Find connection address and verify options
  int opt_index = 0;
  int opt       = getopt_long(argc, argv, short_opt, long_opt, &opt_index);
  switch (opt) {
  case 'h':
  case '?':
    usage();
    return 0;
  default:
    break;
  }

  if (argc - 1 != optind) {
    fprintf(stderr, argc <= optind
                        ? "An address argument must be given.\n"
                        : "Only one address argument may be given.\n");
    usage();
    return 1;
  }

  char *address = argv[optind];
  int   status  = 0;
  opt_index     = 0;
  optind        = 1;
  while ((opt = getopt_long(argc, argv, short_opt, long_opt, &opt_index)) > 0) {
    if (status) {
      break;
    }

    switch (opt) {
    case 'i':
      status = do_get_information(address);
      break;
    case 'd':
      status = do_get_device_status(address);
      break;
    case 'f':
      status = do_get_firmware_version(address);
      break;
    case 's':
      status = do_get_serial_number(address);
      break;
    case 'b':
      status = do_get_battery_level(address);
      break;
    case 'a':
      status = do_get_paired_devices(address);
      break;
    case OPTION_DEVICE_ID:
      status = do_get_device_id(address);
      break;
    case 'n':
      status = do_set_name(address, optarg);
      break;
    case 'o':
      status = do_set_auto_off(address, optarg);
      break;
    case 'c':
      status = do_set_noise_cancelling(address, optarg);
      break;
    case 'l':
      status = do_set_prompt_language(address, optarg);
      break;
    case 'v':
      status = do_set_voice_prompts(address, optarg);
      break;
    case 'p':
      status = do_set_pairing(address, optarg);
      break;
    case OPTION_CONNECT_DEVICE:
      status = do_connect_device(address, optarg);
      break;
    case OPTION_DISCONNECT_DEVICE:
      status = do_disconnect_device(address, optarg);
      break;
    case OPTION_REMOVE_DEVICE:
      status = do_remove_device(address, optarg);
      break;
    case 'e':
      status = do_set_self_voice(address, optarg);
      break;
    case OPTION_SEND_PACKET:
      status = do_send_packet(address, optarg);
      break;
    default:
      status = 1;
    }
  }

  if (status < 0) {
    perror("Error trying to change setting");
  }

  return status;
}

#endif