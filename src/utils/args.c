#include <utils/args.h>
#include <utils/error.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

typedef void (*FlagResolveFunction)(struct Args *);

static void resolve_format_float(struct Args *args);
static void resolve_help(struct Args *args);

struct Flag
{
  char *long_format;
  char *small_format;
  char *description;
  FlagResolveFunction func;
};

#define FLAGS_COUNT 2

static struct Flag flags[] = {
    {"--format-float", "-f", "Formatting as fractions", resolve_format_float},
    {"--help", "-h", "Instructions on how to use", resolve_help} //
};

static int get_expression(struct Args *args, struct Arena *arena, char *str)
{
  size_t buffer_max = 1024;
  char *buffer = (char *)arena_alloc(arena, buffer_max);

  int length = strlen(str);

  while (buffer_max <= length)
  {
    arena_realloc(arena, (void *)buffer, &buffer_max, 1);

    if (buffer_max > 65536)
    {
      errorGeneric("Maximum number of characters reached");
      return 0;
    }
  }

  strncpy(buffer, str, length);
  buffer[length] = '\0';

  args->expression = buffer;

  return 1;
}

enum ArgsResolveStatus resolve_args(struct Args *args, struct Arena *arena, int argc, char *argv[])
{
  args->expression = NULL;

  for (int i = 1; i < argc; i++)
  {
    if (isalnum(argv[i][0]))
    {
      if (!get_expression(args, arena, argv[i]))
        return ARGS_RESOLVE_ERROR;
    }

    if (strncmp(argv[i], "--", 2) == 0 || strncmp(argv[i], "-", 1) == 0)
    {
      int found = 0;
      for (int f = 0; f < FLAGS_COUNT; f++)
      {
        if (strcmp(argv[i], flags[f].long_format) == 0 || strcmp(argv[i], flags[f].small_format) == 0)
        {
          flags[f].func(args);
          found = 1;
          break;
        }
      }

      if (!found)
      {
        errorGeneric("invalid option '%s'.", argv[i]);
        return ARGS_RESOLVE_ERROR;
      }
    }
  }

  if (args->expression == NULL)
    resolve_help(args);

  return ARGS_RESOLVE_SUCCESS;
}

static void resolve_format_float(struct Args *args)
{
  args->format_float = 1;
}

static void resolve_help(struct Args *args)
{
  puts("usage: mathpp <flags> <expression>");
  puts("flags:");

  for (int i = 0; i < FLAGS_COUNT; i++)
  {
    printf("\t%-20s %-4s -> %s\n",
           flags[i].long_format,
           flags[i].small_format,
           flags[i].description);
  }

  exit(EXIT_SUCCESS);
}