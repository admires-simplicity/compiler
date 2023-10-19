#ifndef TYPES_H
#define TYPES_H

typedef enum {
  cintValueType, // this is probably bad (??)
  ui64ValueType,
  StringValueType,
  CustomValueType,
} ValueType;

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
} ExprType;


#endif