#pragma once

#include <utils/arena.h>
#include <stdint.h>

enum ArgsResolveStatus
{
  ARGS_RESOLVE_SUCCESS,
  ARGS_RESOLVE_ERROR
};

struct Args
{
  char *expression;
  uint8_t format_float;
};

enum ArgsResolveStatus resolve_args(struct Args *args, struct Arena *arena, int argc, char *argv[]);
