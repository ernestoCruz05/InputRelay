#include "vdevice.h"
#include <errno.h>
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ioctl_or_die(fd, req, arg)                                             \
  if (ioctl(fd, req, arg) < 0) {                                               \
    perror("ioctl_error");                                                     \
    return -1;                                                                 \
  }

int init_virtual_device(const char *name) {
  int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  if (fd < 0) {
    perror("Could not open /dev/uinput");
    return -1;
  }

  ioctl_or_die(fd, UI_SET_EVBIT, EV_KEY);
  ioctl_or_die(fd, UI_SET_EVBIT, EV_SYN);

  for (int i = KEY_ESC; i <= 120; i++) {
    ioctl_or_die(fd, UI_SET_KEYBIT, i);
  }

  ioctl_or_die(fd, UI_SET_KEYBIT, KEY_SPACE);
  ioctl_or_die(fd, UI_SET_KEYBIT, KEY_ENTER);

  struct uinput_setup usetup;
  memset(&usetup, 0, sizeof(usetup));
  usetup.id.bustype = BUS_USB;
  usetup.id.vendor = 0x1234;
  usetup.id.product = 0x5678;
  snprintf(usetup.name, UINPUT_MAX_NAME_SIZE, "%s", name);

  ioctl_or_die(fd, UI_DEV_SETUP, &usetup);
  ioctl_or_die(fd, UI_DEV_CREATE, 0);

  printf("[+] Virtual Device '%s' created sucessfully (fd:%d)\n", name, fd);
  return fd;
}

void destroy_virtual_device(int fd) {
  if (fd >= 0) {
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
    printf("[-] Virtual Device destroyed \n");
  }
}

int send_event(int fd, int type, int code, int value) {
  struct input_event ie;
  memset(&ie, 0, sizeof(ie));

  ie.type = type;
  ie.code = code;
  ie.value = value;

  if (write(fd, &ie, sizeof(ie)) < 0) {
    perror("write error\n");
    return -1;
  }
  return 0;
}

int send_key_tap(int fd, int key_code) {
  send_event(fd, EV_KEY, key_code, 1);
  send_event(fd, EV_SYN, SYN_REPORT, 0);

  usleep(15000);

  send_event(fd, EV_KEY, key_code, 0);
  send_event(fd, EV_SYN, SYN_REPORT, 0);

  return 0;
}

static int char_to_keycode(char c, int *needs_shift) {
  *needs_shift = 0;

  if (c >= 'A' && c <= 'Z') {
    *needs_shift = 1;
    c = c + 32;
  }

  if (c == '!') {
    *needs_shift = 1;
    return KEY_1;
  }
  if (c == '@') {
    *needs_shift = 1;
    return KEY_2;
  }
  if (c == '#') {
    *needs_shift = 1;
    return KEY_3;
  }

  switch (c) {
  case 'a':
    return KEY_A;
  case 'b':
    return KEY_B;
  case 'c':
    return KEY_C;
  case 'd':
    return KEY_D;
  case 'e':
    return KEY_E;
  case 'f':
    return KEY_F;
  case 'g':
    return KEY_G;
  case 'h':
    return KEY_H;
  case 'i':
    return KEY_I;
  case 'j':
    return KEY_J;
  case 'k':
    return KEY_K;
  case 'l':
    return KEY_L;
  case 'm':
    return KEY_M;
  case 'n':
    return KEY_N;
  case 'o':
    return KEY_O;
  case 'p':
    return KEY_P;
  case 'q':
    return KEY_Q;
  case 'r':
    return KEY_R;
  case 's':
    return KEY_S;
  case 't':
    return KEY_T;
  case 'u':
    return KEY_U;
  case 'v':
    return KEY_V;
  case 'w':
    return KEY_W;
  case 'x':
    return KEY_X;
  case 'y':
    return KEY_Y;
  case 'z':
    return KEY_Z;

  case '1':
    return KEY_1;
  case '2':
    return KEY_2;
  case '3':
    return KEY_3;
  case '4':
    return KEY_4;
  case '5':
    return KEY_5;
  case '6':
    return KEY_6;
  case '7':
    return KEY_7;
  case '8':
    return KEY_8;
  case '9':
    return KEY_9;
  case '0':
    return KEY_0;

  case ' ':
    return KEY_SPACE;
  case '\n':
    return KEY_ENTER;
  case '.':
    return KEY_DOT;
  case ',':
    return KEY_COMMA;
  case '-':
    return KEY_MINUS;
  case '=':
    return KEY_EQUAL;

  default:
    return 0;
  }
}

void type_string(int fd, const char *str) {
  while (*str) {
    int needs_shift = 0;
    int code = char_to_keycode(*str, &needs_shift);

    if (code != 0) {
      if (needs_shift) {
        send_event(fd, EV_KEY, KEY_LEFTSHIFT, 1);
        send_event(fd, EV_SYN, SYN_REPORT, 0);
      }

      send_key_tap(fd, code);

      if (needs_shift) {
        send_event(fd, EV_KEY, KEY_LEFTSHIFT, 0);
        send_event(fd, EV_SYN, SYN_REPORT, 0);
      }

      str++;
      usleep(25000);
    }
  }
}
