#include <core/solver.h>
#include <math.h>
#include <string.h>
#include <utils/print.h>
#include <core/evaluator.h>
#include <stdio.h>
#include <utils/error.h>

struct Poly {
  double complex *coef;
  size_t degree;
  uint8_t is_poly;
};

uint8_t has_variable(struct Expr *expr) {
  switch (expr->kind) {
    case Expr_Equation:  {
      for (int i = 0; i < expr->equation.exprs_len; i++)
        if (has_variable(expr->equation.exprs[i])) return 1;
      return 0;
    }

    case Expr_Binary:
      return has_variable(expr->binary.left) || has_variable(expr->binary.right);
    case Expr_Unary:
      return has_variable(expr->unary.arg);
    case Expr_Call:
      return has_variable(expr->call.arg);
    case Expr_Identifier:
      return 1;
    case Expr_Literal:
      return 0;
  }
}

void find_variable(struct Expr *expr, char *id) { 
  if (expr->kind == Expr_Equation) {
    for (int i = 0; i < expr->equation.exprs_len; i++)
      find_variable(expr->equation.exprs[i], id);
  }

  else if (expr->kind == Expr_Binary) {
    find_variable(expr->binary.left, id);
    find_variable(expr->binary.right, id);
  }

  else if (expr->kind == Expr_Unary)
    find_variable(expr->unary.arg, id);

  else if (expr->kind == Expr_Call)
    find_variable(expr->call.arg, id);

  else if (expr->kind == Expr_Identifier) {
    if (*id == 0) *id = expr->identifier;
    else if (expr->identifier != *id)
      errorGeneric("multiple unknowns are not supported yet.");
  }
}

struct Poly create_poly(struct Arena *arena, size_t degree) {
  struct Poly poly = (struct Poly){
    .coef = arena_alloc(arena, (degree + 1) * sizeof(double complex)),
    .degree = degree,
    .is_poly = 1
  };

  memset(poly.coef, 0, (degree + 1) * sizeof(double complex));
  return poly;
};

struct Poly get_poly(struct Expr *expr, struct Arena *arena) {
  if (expr->kind == Expr_Binary) {
    struct Poly left = get_poly(expr->binary.left, arena);
    struct Poly right = get_poly(expr->binary.right, arena);
    if (!left.is_poly || !right.is_poly) return left.is_poly ? right : left;

    if (expr->binary.op == TOKEN_PLUS || expr->binary.op == TOKEN_MINUS) {
      struct Poly poly = create_poly(arena, left.degree > right.degree ? left.degree : right.degree);
      for (int i = 0; i <= poly.degree; i++) {
        double complex l = i <= left.degree ? left.coef[i] : 0.0;
        double complex r = i <= right.degree ? right.coef[i] : 0.0;
        poly.coef[i] = expr->binary.op == TOKEN_PLUS ? l + r : l - r;
      }
      return poly;
    }

    if (expr->binary.op == TOKEN_ASTERISK) {
      struct Poly poly = create_poly(arena, left.degree + right.degree);
      for (int i = 0; i <= left.degree; i++) {
        for (int j = 0; j <= right.degree; j++)
          poly.coef[i + j] += left.coef[i] * right.coef[j];
      }
      return poly;
    }

    if (expr->binary.op == TOKEN_SLASH) {
      if (right.degree == 0 && cabs(right.coef[0]) > 1e-15) {
        struct Poly poly = create_poly(arena, left.degree);
        for (int i = 0; i <= left.degree; i++)
          poly.coef[i] = left.coef[i] / right.coef[0];
        return poly;
      }
    }

    if (expr->binary.op == TOKEN_CARET) {
      if (right.degree == 0 && fabs(cimag(right.coef[0])) < 1e-9 && creal(right.coef[0]) >= 0 &&
          fabs(creal(right.coef[0]) - round(right.coef[0])) < 1e-9) {
        struct Poly poly = create_poly(arena, 0);
        poly.coef[0] = 1.0;
        for (int i = 0; i < (int)round(creal(right.coef[0])); i++) {
          struct Poly tmp = create_poly(arena, poly.degree + left.degree);
          for (int j = 0; j <= left.degree; j++) {
            for (int k = 0; k <= poly.degree; k++)
              tmp.coef[j + k] += left.coef[j] * poly.coef[k];
          }
          poly = tmp;
        }
        return poly;
      }
    }
  }

  else if (expr->kind == Expr_Unary) {
    struct Poly arg = get_poly(expr->unary.arg, arena);
    if (!arg.is_poly) return arg;
    struct Poly poly = create_poly(arena, arg.degree);
    for (int i = 0; i <= arg.degree; i++)
      poly.coef[i] = -arg.coef[i];
    return poly;
  }

  else if (expr->kind == Expr_Literal) {
    struct Poly poly = create_poly(arena, 0);
    poly.coef[0] = expr->literal;
    return poly;
  }

  else if (expr->kind == Expr_Identifier) {
    struct Poly poly = create_poly(arena, 1);
    poly.coef[1] = 1.0;
    return poly;
  }

  struct Poly poly = create_poly(arena, 0);
  poly.is_poly = 0.0;
  return poly;
}

void print_subscript(size_t n) {
  const char *sub_digits[] = {
    "₀", "₁", "₂", "₃", "₄", "₅", "₆", "₇", "₈", "₉"
  };

  char buf[32];
  snprintf(buf, sizeof(buf), "%zu", n);

  for (size_t i = 0; buf[i] != '\0'; i++) {
    printf("%s", sub_digits[buf[i] - '0']);
  }
}

void solver(struct Program *program, struct Args *args) {
  for (int i = 0; i < program->length; i++) {
     struct Expr *expr = program->exprs[i];
     char id = 0;
  
     if (!has_variable(expr)) {
       double complex result = eval_expr(expr, NULL);

       if (expr->kind == Expr_Equation)
         printf("%s\n", creal(result) > 0.5 ? "true" : "false");
       else print_result(result, args->format_float);
       continue;
     }

     if (expr->kind != Expr_Equation)
       errorGeneric("cannot solve an expression without '='.");

     if (expr->equation.exprs_len != 2)
       errorGeneric("chained equations with variables are not supported.");

     find_variable(expr, &id);
     struct Poly left = get_poly(expr->equation.exprs[0], program->arena);
     struct Poly right = get_poly(expr->equation.exprs[1], program->arena);

     if (!left.is_poly || !right.is_poly)
       errorGeneric("...");

     struct Poly poly = create_poly(program->arena, left.degree > right.degree ? left.degree : right.degree);
     for (int i = 0; i <= poly.degree; i++) {
       double complex l = i <= left.degree ? left.coef[i] : 0.0;
       double complex r = i <= right.degree ? right.coef[i] : 0.0;
       poly.coef[i] = l - r;
     }

     size_t real_degree = 0;
     for (int i = poly.degree; i >= 0; i--) {
       if (cabs(poly.coef[i]) <= 1e-9) continue;
       real_degree = i;
       break;
     }

     if (real_degree == 0) {
       if (cabs(poly.coef[0]) < 1e-9)
         printf("infinite solutions.\n");
       else printf("no solution.\n");
     }

     else if (real_degree == 1) {
       double complex a = poly.coef[1];
       double complex b = poly.coef[0];
       printf("%c = ", id);
       print_result(-b / a, args->format_float);
     }

     else if (real_degree == 2) {
       double complex a = poly.coef[2];
       double complex b = poly.coef[1];
       double complex c = poly.coef[0];
       double complex delta = b * b - 4.0 * a * c;
       double complex r1 = (-b + csqrt(delta)) / (2.0 * a);
       double complex r2 = (-b - csqrt(delta)) / (2.0 * a);
       printf("Δ = ");
       print_result(delta, args->format_float);
       printf("%c", id);
       print_subscript(1);
       printf(" = ");
       print_result(r1, args->format_float);
       printf("%c", id);
       print_subscript(2);
       printf(" = ");
       print_result(r2, args->format_float);
     }

     else {
       for (int j = 0; j < real_degree; j++)
         poly.coef[j] /= poly.coef[real_degree];
       poly.coef[real_degree] = 1.0;
       double complex roots[real_degree];
       for (int j = 0; j < real_degree; j++)
         roots[j] = cpow(0.4 + 0.9 * I, j);
       for (int j = 0; j < 100; j++) {
         for (int k = 0; k < real_degree; k++) {
           double complex p = 0.0;
           for (int l = 0; l <= real_degree; l++)
             p += poly.coef[l] * cpow(roots[k], l);
           double complex den = 1.0;
           for (int l = 0; l < real_degree; l++)
             if (l != k) den *= (roots[k] - roots[l]);
           if (cabs(den) > 1e-15)
             roots[k] -= p / den;
         }
       }
       for (int j = 0; j < real_degree; j++) {
         printf("%c", id);
         print_subscript(j + 1);
         printf(" = ");
         print_result(roots[j], args->format_float);
       }
     }
  }
}
