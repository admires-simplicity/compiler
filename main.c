#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

typedef enum {
  ui64Value
} ValueType;

typedef struct {
  ValueType vtype;
  size_t size;
  uint64_t *octabytes;
} Value;

typedef enum {
  ValueExpr
} ExprType;

typedef enum {
  PlusOp
} Operator;

typedef struct Expr Expr;

struct Expr {
  ExprType etype;
  Operator op;
  size_t noperands;
  Expr *operands;
};

/*
 * vtype -- type of value to make
 * size -- number of octabytes to allocate for *octabytes
 * *values -- ptr to values to set octabytes to
 */
Value *makeValue(ValueType vtype, size_t size, void *values) {
  Value *value = malloc(sizeof (Value));
  value->vtype = vtype;
  if (size > 0 && values != NULL) {
    // allocate space for octabytes and copy from values
    value->octabytes = malloc(size * sizeof (uint64_t));
    if (value->octabytes == NULL) exit(1);
    memcpy(value->octabytes, values, size);
  }
  else if (size > 0 && values == NULL) {
    // allocate space but don't copy
    value->octabytes = malloc(size * sizeof(uint64_t));
    if (value->octabytes == NULL) exit(1);
  }
  else if (size == 0) {
    // don't allow to not allocate an octabytes ptr,
    // so that we can always free octabytes in freeValue.
    exit(1);
  }
  return value;
}

Value *make_ui64Value(uint64_t x) {
  Value *value = makeValue(ui64Value, 1, NULL);
  value->octabytes[0] = x;
  return value;
}

void freeValue(Value *value) {
  free(value->octabytes);
  free(value);
}



/* typedef enum { */
/*   UPLUS, */
/*   UMINUS, */
/* } UnaryOp; */

/* typedef enum { */
/*   BPLUS, */
/*   BMINUS, */
/*   BTIMES, */
/*   BDIV, */
/* } BinaryOp; */

/* typedef struct { */
/*   int value; */
/* } ValueExpr; */

/* typedef struct { */
/*   UnaryOp unaryOp; */
/*   ValueExpr operand; */
/* } UnaryExpr; */

/* typedef struct { */
/*   ValueExpr left; */
/*   BinaryOp binaryOp; */
/*   ValueExpr right; */
/* } BinaryExpr; */



/* int evalValueExpr(ValueExpr vexpr) { */
/*   return vexpr.value; */
/* } */

/* int evalUnaryExpr(UnaryExpr uexpr) { */
/*   int operandValue = evalValueExpr(uexpr.operand); */
/*   switch (uexpr.unaryOp) { */
/*   case UPLUS: */
/*     return operandValue; */
/*   case UMINUS: */
/*     return -operandValue; */
/*   default: */
/*     exit(1); */
/*     break; */
/*   } */
/* } */

/* int evalBinaryExpr(BinaryExpr bexpr) { */
/*   int leftValue = evalValueExpr(bexpr.left); */
/*   int rightValue = evalValueExpr(bexpr.right); */
/*   switch (bexpr.binaryOp) { */
/*   case BPLUS: */
/*     return leftValue + rightValue; */
/*   case BMINUS: */
/*     return leftValue - rightValue; */
/*   case BTIMES: */
/*     return leftValue * rightValue; */
/*   case BDIV: */
/*     return leftValue / rightValue; */
/*   default: */
/*     exit(1); */
/*     break; */
/*   } */
/* } */


int main(int argc, char **argv) {
  
  /* ValueExpr ve1 = { 1 }; */
  /* UnaryExpr ue1 = { UPLUS, { 1 } }; */
  /* BinaryExpr be1 = { { 2 }, BTIMES, { 3 } }; */

  /* printf("ve1 value is %d\n", evalValueExpr(ve1)); */
  /* printf("ue1 value is %d\n", evalUnaryExpr(ue1)); */
  /* printf("be1 value is %d\n", evalBinaryExpr(be1)); */

  Value *value1 = make_ui64Value(12345);

  printf("value1->octabytes[0] == %d\n", value1->octabytes[0]);
  
  freeValue(value1);
  
  return 0;
}
