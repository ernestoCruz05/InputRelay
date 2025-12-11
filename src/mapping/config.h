#ifndef CONFIG_H
#define CONFIG_H

#include "../common.h"

int load_config(const char *filename, RelayConfig *config);
int save_config(const char *filename, RelayConfig *config);
#endif
