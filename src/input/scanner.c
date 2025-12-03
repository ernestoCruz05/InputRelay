#include "scanner.h"
#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define INPUT_DIR "/dev/input/"
#define VDEVICE_NAME "Virtual Keyboard"

int try_open_device(const char *filename, const char *target_name) {
  char path[256];
  snprintf(path, sizeof(path), "%s%s", INPUT_DIR, filename);

  int fd = open(path, O_RDONLY);
  if (fd < 0)
    return -1;

  char name[256] = "Unkown";
  if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0) {
    close(fd);
    return -1;
  }

  if (strcmp(name, VDEVICE_NAME) == 0) {
    close(fd);
    return -1;
  }

  printf("[SCANNER] Checking: %s ('%s')\n", path, name);

  if (strstr(name, target_name) != NULL) {

    int user_asked_specific = (strstr(target_name, "Touchpad") != NULL ||
                               strstr(target_name, "Motion") != NULL);

    if (!user_asked_specific) {
      if (strstr(name, "Touchpad") != NULL) {
        printf("[SCANNER] Skipping Touchpad interface: %s\n", name);
        close(fd);
        return -1;
      }
      if (strstr(name, "Motion") != NULL) {
        printf("[SCANNER] Skipping Motion interface: %s\n", name);
        close(fd);
        return -1;
      }
    }

    printf("[SCANNER] Match found: %s\n", path);
    return fd;
  }

  close(fd);
  return -1;
}

int find_device_by_name(const char *target_name) {
  DIR *dp = opendir(INPUT_DIR);
  if (!dp) {
    perror("Failed to access /dev/input");
    return -1;
  }

  struct dirent *entry;
  while ((entry = readdir(dp))) {
    if (strncmp(entry->d_name, "event", 5) == 0) {
      int fd = try_open_device(entry->d_name, target_name);
      if (fd >= 0) {
        closedir(dp);
        return fd;
      }
    }
  }
  closedir(dp);
  return -1;
}

int grab_device(int fd) {
  if (ioctl(fd, EVIOCGRAB, 1) < 0) {
    perror("Failed to grab device(exclusive access)");
    return -1;
  }
  printf("[SCANNER] Device grabbed exclusively. OS will lose it \n");
  return 0;
}
