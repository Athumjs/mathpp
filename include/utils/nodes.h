#pragma once

#include <stddef.h>
#include <utils/tokens.h>
#include <complex.h>
#include <stdint.h>

enum ExprKind {
  Expr_Equation,
  Expr_Binary,
  Expr_Unary,
  Expr_Call,
  Expr_Literal,
  Expr_Identifier
};

struct Expr {
  enum ExprKind kind;

  union {
    struct {
      struct Expr **exprs;
      size_t exprs_len;
      size_t exprs_cap;
    } equation;

    struct {
      struct Expr *left;
      struct Expr *right;
      enum TokenType op;
    } binary;

    struct {
      struct Expr *arg;
      enum TokenType op;
    } unary;

    struct {
      enum TokenType func;
      double complex attr;
      struct Expr *arg;
    } call;

    double complex literal;
    char identifier;
  };
};
