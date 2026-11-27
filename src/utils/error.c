#include <utils/error.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

void errorGeneric(const char *error, ...) {
  va_list args;
  va_start(args, error);
  printf("\033[31m[ERROR]\033[0m\n ");
  vprintf(error, args);
  putchar('\n');
  va_end(args);
  exit(1);
}
