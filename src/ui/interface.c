#include "../common.h"
#include "../input/scanner.h"
#include "../mapping/config.h"
#include "ncurses.h"
#include <fcntl.h>
#include <linux/input.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int capture_button(int device_fd) {
  if (device_fd < 0)
    return 0;

  struct input_event ev;
  mvprintw(10, 2, "Waiting for button press...");
  refresh();

  int flags = fcntl(device_fd, F_GETFL, 0);
  fcntl(device_fd, F_SETFL, flags | O_NONBLOCK);
  while (read(device_fd, &ev, sizeof(ev)) > 0)
    ;

  fcntl(device_fd, F_SETFL, flags);

  while (1) {
    if (read(device_fd, &ev, sizeof(ev)) > 0) {
      if (ev.type == EV_KEY && ev.value == 1) {
        return ev.code;
      }
    }
    usleep(10000);
  }
}

void run_configurator(const char *device_name) {
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);

  RelayConfig config = {0};
  load_config("config.ini", &config);

  int fd = find_device_by_name(device_name);

  while (1) {
    clear();
    box(stdscr, 0, 0);
    attron(A_BOLD);
    mvprintw(1, 2, "InputRelay Configurator");
    attroff(A_BOLD);

    if (fd < 0) {
      attron(A_COLOR);
      mvprintw(3, 2, "[!] Controller '%s' not found!!", device_name);
      mvprintw(4, 2, "[-] Try re-plugging and restarting");
      mvprintw(6, 2, "[-] Press any key to exit.");
      refresh();
      getch();
      break;
    }
    mvprintw(3, 2, "Current Mappings (%d):", config.mapping_count);
    for (int i = 0; i < config.mapping_count && i < 10; i++) {
      mvprintw(4 + i, 4, "[#] Button %d -> %d steps",
               config.mappings[i].input_code, config.mappings[i].step_count);
    }

    mvprintw(16, 2, "[A] Add New Mapping");
    mvprintw(17, 2, "[S] Save & Exit");
    mvprintw(18, 2, "[Q] Quit without saving");

    int ch = getch();

    if (ch == 'q' || ch == 'Q')
      break;

    if (ch == 's' || ch == 'S') {
      save_config("config.ini", &config);
      break;
    }

    if (ch == 'a' || ch == 'A') {
      clear();
      mvprintw(2, 2, "[+] Capturing Inputs [+]");
      mvprintw(4, 2, "[-] 1. Press the button on ur controller");
      refresh();

      int btn = capture_button(fd);
      mvprintw(6, 2, "[*] Captured code: %d", btn);

      mvprintw(
          8, 2,
          "[!] Just for test, binding 'HELLO' macro to this button"); // TODO:
                                                                      // fix
                                                                      // later!

      // mvprintw(10, 2, "[-] Press any button to confirm!");
      // getch();

      if (config.mapping_count < 64) {
        Mapping *m = &config.mappings[config.mapping_count++];

        m->input_code = btn;
        m->step_count = 0;

        m->steps[m->step_count++] = (Action){ACTION_KEY, KEY_H, 1, 0};
        m->steps[m->step_count++] = (Action){ACTION_KEY, KEY_E, 1, 0};
        m->steps[m->step_count++] = (Action){ACTION_KEY, KEY_L, 1, 0};
        m->steps[m->step_count++] = (Action){ACTION_KEY, KEY_L, 1, 0};
        m->steps[m->step_count++] = (Action){ACTION_KEY, KEY_O, 1, 0};
      }
    }
  }
  endwin();
  if (fd >= 0)
    close(fd);
}
