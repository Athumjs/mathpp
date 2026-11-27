#include <utils/print.h>
#include <math.h>
#include <stdio.h>

uint8_t to_fraction(double number, int64_t *num, int64_t *den) {
  if (isnan(number) || isinf(number)) return 0;
  int sign = (number < 0.0f) ? -1 : 1;
  number = fabs(number);

  if (fabs(number - round(number)) < 1e-9) {
    *num = sign * (int64_t)round(number);
    *den = 1;
    return 1;
  }

  int64_t current_num = 1;
  int64_t old_num = 0;
  int64_t current_den = 0;
  int64_t old_den = 1;
  double n = number;

  while (1) {
    int64_t a = floor(n);
    *num = a * current_num + old_num;
    *den = a * current_den + old_den;

    if (*den > 10000) return 0;
    if (number - (double)*num / (double)*den < 1e-9) {
      *num *= sign;
      return 1;
    }

    old_num = current_num;
    old_den = current_den;
    current_num = *num;
    current_den = *den;
    double rest = n - a;
    if (rest < 1e-12) break;
    n = 1.0 / rest;
  }

  return 0;
}

void format_number(double number, uint8_t format_float) {
  int64_t num, den;
  if (format_float && to_fraction(number, &num, &den) && den > 1)
    printf("%ld/%ld", num, den);
  else printf("%g", number);
} 

void print_result(double complex result, uint8_t format_float) {
  double re = creal(result);
  double im = cimag(result);

  if (fabs(im) < 1e-9) {
    format_number(re, format_float);
    printf("\n");
  } else if (fabs(re) < 1e-9) {
    format_number(im, format_float);
    printf("i\n");
  } else {
    format_number(re, format_float);
    printf(" %c ", im < 0 ? '-' : '+');
    format_number(fabs(im), format_float);
    printf("i\n");
  }
}
