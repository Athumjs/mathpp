#include <core/lexer.h>
#include <stdint.h>
#include <stdlib.h>
#include <utils/error.h>
#include <string.h>
#include <ctype.h>

void addTokenNumber(double complex value, struct Tokens *tokens, struct Arena *arena) {
  if (tokens->length == tokens->capacity)
    tokens->tokens = arena_realloc(arena, tokens->tokens, &tokens->capacity, sizeof(struct Token));
  tokens->tokens[tokens->length++] = (struct Token){ .type = TOKEN_NUMBER, .tkv.number = value };
}

void addTokenOther(enum TokenType type, const char *value, struct Tokens *tokens, struct Arena *arena) {
  if (tokens->length == tokens->capacity)
    tokens->tokens = arena_realloc(arena, tokens->tokens, &tokens->capacity, sizeof(struct Token));
  tokens->tokens[tokens->length++] = (struct Token){ .type = type, .tkv.value = value };
}

void addTokenFunc(enum TokenType type, double complex value, struct Tokens *tokens, struct Arena *arena) {
  if (tokens->length == tokens->capacity)
    tokens->tokens = arena_realloc(arena, tokens->tokens, &tokens->capacity, sizeof(struct Token));
  tokens->tokens[tokens->length++] = (struct Token){ .type = type, .tkv.suffix = value };
}

void addTokenId(char ch, struct Tokens *tokens, struct Arena *arena) {
  if (tokens->length == tokens->capacity)
    tokens->tokens = arena_realloc(arena, tokens->tokens, &tokens->capacity, sizeof(struct Token));
  tokens->tokens[tokens->length++] = (struct Token){ .type = TOKEN_IDENTIFIER, .tkv.id = ch };
}

double parse_number(const char *expression, int *index) {
  char *endptr;
  double num = strtod(&expression[*index], &endptr);
  *index = (int)(endptr - expression);
  return num;
}

void lexer(const char *expression, struct Tokens *tokens, struct Arena *arena) {
  int index = 0;
  int len = strlen(expression);

  while (index < len) {
    char ch = expression[index];

    if (ch == ' ' || ch == '\n') {
      index++;
      continue;
    }

    if (isdigit(ch)) {
      addTokenNumber(parse_number(expression, &index), tokens, arena);
      continue;
    } 

    if (isalpha(ch)) {
      int start = index;
      while (isalpha(ch)) ch = expression[++index];
      int word_len = index - start;

#define X(name, str) \
      if (word_len == strlen(str) && memcmp(expression + start, str, word_len) == 0) {\
        addTokenOther(name, str, tokens, arena); \
        continue; \
      }
      CONSTANTS
#undef X

#define X(name, str) \
        if (word_len == strlen(str) && memcmp(expression + start, str, word_len) == 0) {\
          double suffix = isdigit(expression[index]) ? parse_number(expression, &index) : -1; \
          addTokenFunc(name, suffix, tokens, arena); \
          continue; \
        }
        FUNCTIONS
#undef X

      for (int i = 0; i < word_len; i++)
        addTokenId(expression[start + i], tokens, arena);
      continue;
    }

    const char *best = NULL;
    enum TokenType bestType;
    size_t bestLen = 0;

#define X(name, str)                                                           \
  if (sizeof(str) - 1 > bestLen &&                                             \
      memcmp(expression + index, str, sizeof(str) - 1) == 0) {                       \
    best = str;                                                                \
    bestType = name;                                                           \
    bestLen = sizeof(str) - 1;                                                 \
  }
    OPERATORS
    SYMBOLS
#undef X

    if (best) {
      addTokenOther(bestType, best, tokens, arena);
      for (int i = 0; i < bestLen; i++)
        index++;
      continue;
    }

    errorGeneric("invalid character '%c'.", ch);
  }

  addTokenOther(TOKEN_EOF, NULL, tokens, arena);
}
