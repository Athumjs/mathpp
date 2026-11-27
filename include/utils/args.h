#pragma once

#include <utils/arena.h>
#include <stdint.h>

struct Args {
  char *expression;
  uint8_t format_float;
};

struct Args resolve_args(int argc, char *argv[], struct Arena *arena);
