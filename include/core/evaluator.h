#pragma once

#include <utils/args.h>
#include <core/parser.h>

struct Variable {
  char identifier;
  double complex value;
};

struct Context {
  struct Variable *vars;
  size_t vars_len;
  size_t vars_cap;
};

void evaluator(struct Program *program, struct Args *args);
double complex eval_expr(struct Expr *expr, struct Context *ctx);
