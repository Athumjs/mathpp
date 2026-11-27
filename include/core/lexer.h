#pragma once

#include <utils/arena.h>
#include <utils/tokens.h>

struct Tokens {
  struct Token *tokens;
  size_t length;
  size_t capacity;
};

void lexer(const char *expression, struct Tokens *tokens, struct Arena *arena);
