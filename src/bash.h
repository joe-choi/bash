#pragma once
#ifndef __BASH__H
#define __BASH__H
#include <pebble.h>
void handle_init (void);
void handle_deinit(void);

enum Actions {
  UP=0,
  MIDDLE=1,
  DOWN=2,
  SHAKE=3
};

#endif