#ifndef VDEVICE_H
#define VDEVICE_H

int init_virtual_device(const char *name);

void destroy_virtual_device(int fd);

int send_event(int fd, int type, int code, int value);

int send_key_tap(int fd, int key_code);

void type_string(int fd, const char *str);

#endif // !VDEVICE_H
