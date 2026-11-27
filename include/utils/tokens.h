#pragma once

#include <complex.h>

#define FUNCTIONS \
  X(TOKEN_FUNC_SIN, "sin") \
  X(TOKEN_FUNC_COS, "cos") \
  X(TOKEN_FUNC_TAN, "tan") \
  X(TOKEN_FUNC_LOG, "log") \
  X(TOKEN_FUNC_LN, "ln") \
  X(TOKEN_FUNC_SQRT, "sqrt") \
  X(TOKEN_FUNC_ROOT, "root")

#define CONSTANTS \
  X(TOKEN_CONST_I, "i") \
  X(TOKEN_CONST_E, "e") \
  X(TOKEN_CONST_PI, "pi")

#define OPERATORS \
  X(TOKEN_PLUS, "+") \
  X(TOKEN_MINUS, "-") \
  X(TOKEN_ASTERISK, "*") \
  X(TOKEN_SLASH, "/") \
  X(TOKEN_CARET, "^")

#define SYMBOLS \
  X(TOKEN_LPAREN, "(") \
  X(TOKEN_RPAREN, ")") \
  X(TOKEN_EQUAL, "=") \
  X(TOKEN_SEMICOLON, ";")

#define OTHERS \
  X(TOKEN_NUMBER, "number") \
  X(TOKEN_IDENTIFIER, "identifier")

enum TokenType {
#define X(name, str) name,
  FUNCTIONS
  CONSTANTS
  OPERATORS
  SYMBOLS
  OTHERS
#undef X
  TOKEN_EOF
};

static const char *tk_names[] = {
#define X(name, str) [name] = str,
  FUNCTIONS
  CONSTANTS
  OPERATORS
  SYMBOLS
  OTHERS
#undef X
};

struct Token {
  enum TokenType type;
  
  union TokenValue {
    char id;
    const char *value;
    double complex suffix;
    double complex number;
  } tkv;
};
