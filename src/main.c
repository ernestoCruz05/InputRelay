#include "common.h"
#include "input/scanner.h"
#include "mapping/config.h"
#include "mapping/translator.h"
#include "output/vdevice.h"
#include "ui/interface.h"
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define VDEVICE_NAME "Virtual Keyboard"

static volatile int keep_running = 1;

void handle_signal(int sig) {
  (void)sig;
  keep_running = 0;
}

void add_key_step(Mapping *m, int key_code) {
  if (m->step_count >= MAX_MACRO_ACTIONS)
    return;

  m->steps[m->step_count].type = ACTION_KEY;
  m->steps[m->step_count].code = key_code;
  m->steps[m->step_count].value = 1;
  m->step_count++;
}

void add_wait_step(Mapping *m, int ms) {
  if (m->step_count >= MAX_MACRO_ACTIONS)
    return;

  m->steps[m->step_count].type = ACTION_WAIT;
  m->steps[m->step_count].duration_ms = ms;
  m->step_count++;
}

void daemonize() {
  pid_t pid = fork();

  if (pid < 0)
    exit(EXIT_FAILURE);
  if (pid > 0)
    exit(EXIT_SUCCESS);

  if (setsid() < 0)
    exit(EXIT_FAILURE);

  pid = fork();
  if (pid < 0)
    exit(EXIT_FAILURE);
  if (pid > 0)
    exit(EXIT_SUCCESS);

  chdir("/");

  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
}

int main(int argc, char *argv[]) {
  printf("InputRelay v2.0\n");

  if (geteuid() != 0) {
    fprintf(stderr, "Error: Must run as root to access hardware devices\n Run "
                    "with sudo.\n");
    return 1;
  }

  if (argc > 1 && strcmp(argv[1], "--daemon") == 0) {
    printf("[*] Starting the Relay as 'daemon'\n");
    daemonize();
    // TODO: Add sysloging
  }

  if (argc > 1 && strcmp(argv[1], "--configure") == 0) {
    const char *dev = (argc > 2) ? argv[2] : "Xbox";
    run_configurator(dev);
    return 0;
  }

  RelayConfig config = {0};

  const char *target_name = (argc > 1) ? argv[1] : "Xbox";

  signal(SIGINT, handle_signal);

  printf("[*] Loading config.ini...\n");
  if (load_config("config.ini", &config) != 0) {
    return 1;
  }

  printf("[*] Starting input Virtual Device\n");
  int vfd = init_virtual_device(VDEVICE_NAME);

  if (vfd < 0)
    return 1;

  sleep(1);

  printf("[*] Loaded %d mappings \n", config.mapping_count);

  printf("[*] Scanning for device name containing: '%s...'\n", target_name);
  int input_fd = find_device_by_name(target_name);

  if (input_fd < 0) {
    fprintf(stderr, "[!] Could not find controller.\n");
    return 1;
  }

  if (grab_device(input_fd) < 0)
    return 1;

  printf("[*] Relay active. Press Ctrl+c to stop\n");

  struct input_event ev;
  while (keep_running) {
    int n = read(input_fd, &ev, sizeof(ev));

    if (n < sizeof(ev))
      break;

    if (ev.type == EV_KEY && ev.value == 1) {
      printf("[INPUT] Code: &d\n", ev.code);

      for (int i = 0; i < config.mapping_count; i++) {
        if (config.mappings[i].input_code == ev.code) {
          execute_mapping(vfd, &config.mappings[i]);
        }
      }
    }
  }

  /*
  // Macro testing...
  printf("[*] Simulation BTN_SOUTH press\n");

  for (int i = 0; i < config.mapping_count; i++) {
    if (config.mappings[i].input_code == BTN_SOUTH) {
      execute_mapping(vfd, &config.mappings[i]);
    }
  }
*/

  /*
  // Phrase output test
    printf("[*] Output test: \n");
    sleep(1);

    type_string(vfd, "My final message, Goodbye!");

    send_key_tap(vfd, KEY_ENTER);
  */
  printf("[*] Finished! Cleaning up. \n");
  destroy_virtual_device(vfd);
  close(input_fd);

  if (config.mappings)
    free(config.mappings);

  return 0;
}
