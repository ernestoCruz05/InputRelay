#include "config.h"
#include "../common.h"
#include <ctype.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN 1024

typedef struct {
  const char *name;
  int code;
} KeyNameMap;

static const KeyNameMap key_lookup[] = {{"KEY_A", KEY_A},
                                        {"KEY_B", KEY_B},
                                        {"KEY_C", KEY_C},
                                        {"KEY_D", KEY_D},
                                        {"KEY_E", KEY_E},
                                        {"KEY_F", KEY_F},
                                        {"KEY_G", KEY_G},
                                        {"KEY_H", KEY_H},
                                        {"KEY_I", KEY_I},
                                        {"KEY_J", KEY_J},
                                        {"KEY_K", KEY_K},
                                        {"KEY_L", KEY_L},
                                        {"KEY_M", KEY_M},
                                        {"KEY_N", KEY_N},
                                        {"KEY_O", KEY_O},
                                        {"KEY_P", KEY_P},
                                        {"KEY_Q", KEY_Q},
                                        {"KEY_R", KEY_R},
                                        {"KEY_S", KEY_S},
                                        {"KEY_T", KEY_T},
                                        {"KEY_U", KEY_U},
                                        {"KEY_V", KEY_V},
                                        {"KEY_W", KEY_W},
                                        {"KEY_X", KEY_X},
                                        {"KEY_Y", KEY_Y},
                                        {"KEY_Z", KEY_Z},
                                        {"KEY_SPACE", KEY_SPACE},
                                        {"KEY_ENTER", KEY_ENTER},
                                        {"KEY_ESC", KEY_ESC},
                                        {"KEY_TAB", KEY_TAB},
                                        {"BTN_SOUTH", BTN_SOUTH},
                                        {"BTN_EAST", BTN_EAST},
                                        {"BTN_NORTH", BTN_NORTH},
                                        {"BTN_WEST", BTN_WEST},
                                        {NULL, 0}};

int get_code_from_name(char *name) {
  while (isspace(*name))
    name++;

  char *end = name + strlen(name) - 1;
  while (end > name && isspace(*end))
    *end-- = 0;

  if (strncmp(name, "WAIT_", 5) == 0) {
    return -atoi(name + 5);
  }

  for (int i = 0; key_lookup[i].name != NULL; i++) {
    if (strcmp(name, key_lookup[i].name) == 0) {
      return key_lookup[i].code;
    }
  }
  return 0;
}

const char *get_name_from_code(int code) {
  if (code < 0) {
    static char wait_buf[32];
    snprintf(wait_buf, sizeof(wait_buf), "WAIT_%d", -code);
    return wait_buf;
  }

  for (int i = 0; key_lookup[i].name != NULL; i++) {
    if (key_lookup[i].code == code)
      return key_lookup[i].name;
  }

  return "UNKOWN";
}

// TODO: Make this faster later on, for now can be O(n^2)
int save_config(const char *filename, RelayConfig *config) {
  FILE *f = fopen(filename, "w");
  if (!f)
    return -1;

  fprintf(f, "# RELAY CONFIG \n [MAPPINGS]\n");

  for (int i = 0; i < config->mapping_count; i++) {
    Mapping *m = &config->mappings[i];

    fprintf(f, "%s = ", get_name_from_code(m->input_code));

    for (int j = 0; j < m->step_count; j++) {
      Action *act = &m->steps[j];
      int val = (act->type == ACTION_WAIT)
                    ? -act->duration_ms
                    : act->code; // REMIND Nao vai funcionar mas whatever, da
                                 // fix depois

      fprintf(f, "%s", get_name_from_code(val));
      if (j < m->step_count - 1)
        fprintf(f, ", ");
    }
    fprintf(f, "\n");
  }
  fclose(f);
  printf("[CONFIG] Saved %d mappings to the file %s\n", config->mapping_count,
         filename);
  return 0;
}

int load_config(const char *filename, RelayConfig *config) {
  FILE *f = fopen(filename, "r");
  if (!f) {
    perror("Failed to open the config file");
    return -1;
  }
  config->mappings = malloc(sizeof(Mapping) * 64);
  config->mapping_count = 0;

  char line[MAX_LINE_LEN];
  while (fgets(line, sizeof(line), f)) {
    if (line[0] == '#' || line[0] == '[' || line[0] == '\n')
      continue;

    char *trigger_str = strtok(line, "=");
    char *action_str = strtok(NULL, "\n");

    if (!trigger_str || !action_str)
      continue;

    int trigger_code = get_code_from_name(trigger_str);
    if (trigger_code <= 0) {
      printf("[CONFIG] Unkown trigger '%s' \n", trigger_str);
      continue;
    }

    Mapping *m = &config->mappings[config->mapping_count++];
    m->input_code = trigger_code;
    m->step_count = 0;

    char *action_token = strtok(action_str, ",");
    while (action_token != NULL && m->step_count < MAX_MACRO_ACTIONS) {
      int code = get_code_from_name(action_token);

      Action *act = &m->steps[m->step_count++];

      if (code < 0) {
        act->type = ACTION_WAIT;
        act->duration_ms = -code;
      } else {
        act->type = ACTION_KEY;
        act->code = code;
        act->value = 1;
      }
      action_token = strtok(NULL, ",");
    }
    printf("[CONFIG] Loaded: %s -> %d actions \n", trigger_str, m->step_count);
  }
  fclose(f);
  return 0;
}
