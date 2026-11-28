#include <core/solver.h>
#include <core/evaluator.h>
#include <utils/error.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  struct Arena arena = {0};
  arena.current = arenaBlock_create(4096);

  struct Args args;
  if (resolve_args(&args, &arena, argc, argv) == ARGS_RESOLVE_ERROR)
    exit(EXIT_FAILURE);

  struct Tokens tokens = (struct Tokens){
      .tokens = arena_alloc(&arena, sizeof(struct Token)),
      .capacity = 1,
      .length = 0};

  struct Program program = (struct Program){
      .arena = &arena,
      .exprs = arena_alloc(&arena, sizeof(struct Expr *)),
      .capacity = 1,
      .length = 0};

  lexer(args.expression, &tokens, &arena);
  parser(&tokens, &program);

  if (!program.has_variable)
    evaluator(&program, &args);
  else
    solver(&program, &args);

  arena_destroy(&arena);
  return 0;
}
