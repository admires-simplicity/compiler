#ifndef EXPR_H
#define EXPR_H

#include <stddef.h>

//#include "types.h"

// typedef struct Expr Expr;

// struct Expr {
//   ExprType etype;
//   size_t subexpr_count;
//   void *subexprs;
// };

typedef enum {
  NullExpr,
  ValueExpr,
  BinAddExpr,
  ExprListExpr,
  CallExpr,
  StatementExpr,
  BlockExpr,
  FunctionDeclExpr, // at some point I will probably want to split this into FunctionDeclExpr and FunctionDefExpr
  TypeExpr,
  VariableDeclExpr,
  IdentifierExpr,
  GroupingExpr,
  ReturnExpr,
  EqEqExpr, // x == y
  ConditionalExpr,
  AssignmentExpr,
  TypedExprExpr,
} ExprType;

typedef struct {
  ExprType etype;
  size_t subexpr_count;
  void *subexprs;
} Expr;

#endif