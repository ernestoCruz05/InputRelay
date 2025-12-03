#ifndef COMMON_H
#define COMMON_H

#include <linux/input.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_MACRO_ACTIONS 16

typedef enum {
  ACTION_KEY,   // Standard key
  ACTION_MOUSE, // Mouse actions
  ACTION_WAIT   // Pause execution (macro only)
} ActionType;

typedef struct {
  ActionType type;
  int code;        // Linux event code read from inputs
  int value;       // 1= Press, etc...
  int duration_ms; // For macros
} Action;

typedef struct {
  int input_code;
  Action steps[MAX_MACRO_ACTIONS];
  int step_count;
} Mapping;

typedef struct {
  char device_path[256];
  Mapping *mappings;
  int mapping_count;
} RelayConfig;

#endif // !COMMON_H
