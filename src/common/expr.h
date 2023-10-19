#ifndef EXPR_H
#define EXPR_H

#include <stddef.h>

#include "types.h"

// typedef struct Expr Expr;

// struct Expr {
//   ExprType etype;
//   size_t subexpr_count;
//   void *subexprs;
// };

typedef struct {
  ExprType etype;
  size_t subexpr_count;
  void *subexprs;
} Expr;

#endif