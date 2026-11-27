#include <utils/args.h>
#include <utils/error.h>
#include <string.h>
#include <ctype.h>

struct Args resolve_args(int argc, char **argv, struct Arena *arena) {
  struct Args args = {0};
  int index = 1;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--format-float") == 0 || strcmp(argv[i], "-f") == 0)
      args.format_float = 1;

    else if (strncmp(argv[i], "--", 2) == 0)
      errorGeneric("invalid option '%s'.", argv[i]);

    else break;
    index++;
  }

  char *buffer = arena_alloc(arena, 1025);
  size_t buffer_cap = 1025;
  size_t buffer_len = 1;
  buffer[0] = '\0';

  for (int i = index; i < argc; i++) {
    buffer_len += strlen(argv[i]) + 2;
    if (buffer_len == buffer_cap)
      buffer = arena_realloc(arena, buffer, &buffer_cap, 1);
    if (i > index)
      strcat(buffer, " ");
    strcat(buffer, argv[i]);
  }

  args.expression = buffer;
  return args;
}
