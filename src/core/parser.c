#include <core/parser.h>
#include <string.h>
#include <utils/error.h>

enum TokenType peek(int i, struct Tokens *tokens) {
  return tokens->tokens[i].type;
}

#define PEEK() peek(*i, tokens)

union TokenValue consume(int *i, struct Tokens *tokens, enum TokenType type) {
  if (PEEK() != type)
    errorGeneric("expected %s.", tk_names[type]);
  (*i)++;
  return tokens->tokens[*i - 1].tkv;
}

#define CONSUME(t) consume(i, tokens, t)

struct Expr *new_expr(int *i, struct Tokens *tokens, struct Arena *arena, enum ExprKind kind) {
  struct Expr *expr = arena_alloc(arena, sizeof(struct Expr));
  memset(expr, 0, sizeof(struct Expr));
  expr->kind = kind;
  return expr;
}

struct Expr *parseExpr(int *i, struct Tokens *tokens, struct Program *program);
struct Expr *exprUnary(int *i, struct Tokens *tokens, struct Program *program);

uint8_t is_call(enum TokenType type) {
#define X(name, str) \
  if (type == name) \
    return 1;
  FUNCTIONS
#undef X
  return 0;
}

struct Expr *exprPrimary(int *i, struct Tokens *tokens, struct Program *program) {
  if (is_call(PEEK())) {
    struct Expr *expr = new_expr(i, tokens, program->arena, Expr_Call);
    expr->call.func = PEEK();
    expr->call.attr = CONSUME(PEEK()).suffix;
    expr->call.arg = exprUnary(i, tokens, program);
    return expr;
  }

  if (PEEK() == TOKEN_LPAREN) {
    CONSUME(TOKEN_LPAREN);
    struct Expr *expr = parseExpr(i, tokens, program);
    CONSUME(TOKEN_RPAREN);
    return expr;
  }

  else if (PEEK() == TOKEN_NUMBER) {
    struct Expr *expr = new_expr(i, tokens, program->arena, Expr_Literal);
    expr->literal = CONSUME(TOKEN_NUMBER).number;
    return expr;
  }

  else if (PEEK() == TOKEN_CONST_I) {
    CONSUME(TOKEN_CONST_I);
    struct Expr *expr = new_expr(i, tokens, program->arena, Expr_Literal);
    expr->literal = I;
    return expr;
  }

  else if (PEEK() == TOKEN_CONST_E) {
    CONSUME(TOKEN_CONST_E);
    struct Expr *expr = new_expr(i, tokens, program->arena, Expr_Literal);
    expr->literal = 2.71828182845904523536;
    return expr;
  }

  else if (PEEK() == TOKEN_CONST_PI) {
    CONSUME(TOKEN_CONST_PI);
    struct Expr *expr = new_expr(i, tokens, program->arena, Expr_Literal);
    expr->literal = 3.14159265358979323846;
    return expr;
  }

  else if (PEEK() == TOKEN_IDENTIFIER) {
    struct Expr *expr = new_expr(i, tokens, program->arena, Expr_Identifier);
    expr->identifier = CONSUME(TOKEN_IDENTIFIER).id;
    program->has_variable = 1;
    return expr;
  }

  errorGeneric("expected expression.");
}

struct Expr *exprPower(int *i, struct Tokens *tokens, struct Program *program) {
  struct Expr *expr = exprPrimary(i, tokens, program);

  if (PEEK() == TOKEN_CARET) {
    struct Expr *tmp = new_expr(i, tokens, program->arena, Expr_Binary);
    tmp->binary.left = expr;
    tmp->binary.op = TOKEN_CARET;
    CONSUME(TOKEN_CARET);
    tmp->binary.right = exprPower(i, tokens, program);
    expr = tmp;
  }

  return expr;
}

struct Expr *exprUnary(int *i, struct Tokens *tokens, struct Program *program) {
  if (PEEK() == TOKEN_MINUS) {
    struct Expr *expr = new_expr(i, tokens, program->arena, Expr_Unary);
    expr->unary.op = PEEK();
    CONSUME(PEEK());
    expr->unary.arg = exprUnary(i, tokens, program);
    return expr;
  }
  
  return exprPower(i, tokens, program);
}

uint8_t can_mul_implicit(enum TokenType type) {
  return type == TOKEN_IDENTIFIER || type == TOKEN_CONST_I ||
    type == TOKEN_CONST_E || type == TOKEN_CONST_PI ||
    type == TOKEN_LPAREN;
}

struct Expr *exprBinaryGreater(int *i, struct Tokens *tokens, struct Program *program) {
  struct Expr *expr = exprUnary(i, tokens, program);

  while (PEEK() == TOKEN_ASTERISK || PEEK() == TOKEN_SLASH || can_mul_implicit(PEEK())) {
    struct Expr *tmp = new_expr(i, tokens, program->arena, Expr_Binary);
    tmp->binary.left = expr;

    if (PEEK() == TOKEN_ASTERISK || PEEK() == TOKEN_SLASH) {
      tmp->binary.op = PEEK();
      CONSUME(PEEK());
    } else tmp->binary.op = TOKEN_ASTERISK;

    tmp->binary.right = exprUnary(i, tokens, program);
    expr = tmp;
  }

  return expr;
}

struct Expr *exprBinaryLess(int *i, struct Tokens *tokens, struct Program *program) {
  struct Expr *expr = exprBinaryGreater(i, tokens, program);

  while (PEEK() == TOKEN_PLUS || PEEK() == TOKEN_MINUS) {
    struct Expr *tmp = new_expr(i, tokens, program->arena, Expr_Binary);
    tmp->binary.left = expr;
    tmp->binary.op = PEEK();
    CONSUME(PEEK());
    tmp->binary.right = exprBinaryGreater(i, tokens, program);
    expr = tmp;
  }

  return expr;
}

struct Expr *exprEquation(int *i, struct Tokens *tokens, struct Program *program) {
  struct Expr *expr = exprBinaryLess(i, tokens, program);

  if (PEEK() == TOKEN_EQUAL) {
    struct Expr *tmp = new_expr(i, tokens, program->arena, Expr_Equation);
    tmp->equation.exprs = arena_alloc(program->arena, 2 * sizeof(struct Expr *));
    tmp->equation.exprs_cap = 2;
    tmp->equation.exprs_len = 1;
    tmp->equation.exprs[0] = expr;

    while (PEEK() == TOKEN_EQUAL) {
      CONSUME(TOKEN_EQUAL);
      if (tmp->equation.exprs_len == tmp->equation.exprs_cap)
        tmp->equation.exprs = arena_realloc(program->arena, tmp->equation.exprs, &tmp->equation.exprs_cap, sizeof(struct Expr *));
      tmp->equation.exprs[tmp->equation.exprs_len++] = exprBinaryLess(i, tokens, program);
    }

    expr = tmp;
  }

  return expr;
}

struct Expr *parseExpr(int *i, struct Tokens *tokens, struct Program *program) {
  return exprEquation(i, tokens, program);
}

void parser(struct Tokens *tokens, struct Program *program) {
  int index = 0;

  while (peek(index, tokens) != TOKEN_EOF) {
    if (program->length == program->capacity)
      program->exprs = arena_realloc(program->arena, program->exprs, &program->capacity, sizeof(struct Expr *));
    program->exprs[program->length++] = parseExpr(&index, tokens, program);

    if (peek(index, tokens) == TOKEN_SEMICOLON)
      index++;
  }
}
