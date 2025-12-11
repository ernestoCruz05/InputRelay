#include "translator.h"
#include "../output/vdevice.h"
#include <stdio.h>
#include <unistd.h>

void execute_action(int vfd, Action *act) {
  switch (act->type) {
  case ACTION_KEY:
    // Placeholder btw
    send_key_tap(vfd, act->code);
    break;
  case ACTION_WAIT:
    printf("... waiting %d ms ...\n", act->duration_ms);
    usleep(act->duration_ms * 1000);
    break;
  case ACTION_MOUSE:
    break;
  }
}

void execute_mapping(int vfd, Mapping *map) {
  if (!map)
    return;

  printf("[^] Testing test macro\n");

  for (int i = 0; i < map->step_count; i++) {
    execute_action(vfd, &map->steps[i]);
  }
}
