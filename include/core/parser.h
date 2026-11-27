#pragma once

#include <core/lexer.h>
#include <utils/nodes.h>

struct Program {
  struct Arena *arena;
  struct Expr **exprs;
  size_t length;
  size_t capacity;
  uint8_t has_variable;
};

void parser(struct Tokens *tokens, struct Program *program);
