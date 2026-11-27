#include <core/evaluator.h>
#include <math.h>
#include <utils/print.h>
#include <stdio.h>

double complex eval_equation(struct Expr *expr, struct Context *ctx) {
  double complex first = eval_expr(expr->equation.exprs[0], ctx);

  for (int i = 1; i < expr->equation.exprs_len; i++) {
    double complex current = eval_expr(expr->equation.exprs[i], ctx);
    if (cabs(first - current) >= 1e-9)
      return 0.0;
  }

  return 1.0;
}

double complex eval_binary(struct Expr *expr, struct Context *ctx) {
  double complex left = eval_expr(expr->binary.left, ctx);
  double complex right  = eval_expr(expr->binary.right, ctx);

  if (expr->binary.op == TOKEN_PLUS)
    return left + right;
  else if (expr->binary.op == TOKEN_MINUS)
    return left - right;
  else if (expr->binary.op == TOKEN_ASTERISK)
    return left * right;
  else if (expr->binary.op == TOKEN_SLASH)
    return left / right;
  else return cpow(left, right);
}

double complex eval_unary(struct Expr *expr, struct Context *ctx) {
  double complex arg = eval_expr(expr->unary.arg, ctx);
  double re = -creal(arg);
  double im = -cimag(arg);
  if (fabs(im) < 1e-15) im = 0.0;
  return re + im * I;
}

double complex eval_call(struct Expr *expr, struct Context *ctx) {
  double complex arg = eval_expr(expr->call.arg, ctx);

  if (expr->call.func == TOKEN_FUNC_SIN)
    return csin(arg);
  else if (expr->call.func == TOKEN_FUNC_COS)
    return ccos(arg);
  else if (expr->call.func == TOKEN_FUNC_TAN)
    return ctan(arg);
  else if (expr->call.func == TOKEN_FUNC_LOG)
    return clog(arg) / clog(expr->call.attr);
  else if (expr->call.func == TOKEN_FUNC_LN)
    return clog(arg);
  else if (expr->call.func == TOKEN_FUNC_SQRT)
    return csqrt(arg);
  else return cpow(arg, 1.0 / expr->call.attr);
}

double complex eval_expr(struct Expr *expr, struct Context *ctx) {
  switch (expr->kind) {
    case Expr_Equation:
      return eval_equation(expr, ctx);
    case Expr_Binary:
      return eval_binary(expr, ctx);
    case Expr_Unary:
      return eval_unary(expr, ctx);
    case Expr_Call:
      return eval_call(expr, ctx);
    case Expr_Literal:
      return expr->literal;
  }
}

void evaluator(struct Program *program, struct Args *args) {
  for (int i = 0; i < program->length; i++) {
    struct Expr *expr = program->exprs[i];
    double complex result = eval_expr(expr, NULL);

    if (expr->kind == Expr_Equation)
      printf("%s\n", creal(result) > 0.5 ? "true" : "false");
    else print_result(result, args->format_float);
  }
}
