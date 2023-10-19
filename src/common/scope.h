#ifndef SCOPE_H
#define SCOPE_H

#include <stddef.h>

#include "expr.h"

typedef struct {
  size_t expr_count;
  size_t expr_space;
  Expr **exprs;
} Scope;

#endif