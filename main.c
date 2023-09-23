#include <stdio.h>
#include <stdlib.h>

typedef enum {
  UPLUS,
  UMINUS,
} UnaryOp;

typedef enum {
  BPLUS,
  BMINUS,
  BTIMES,
  BDIV,
} BinaryOp;

typedef struct {
  int value;
} ValueExpr;

typedef struct {
  UnaryOp unaryOp;
  ValueExpr operand;
} UnaryExpr;

typedef struct {
  ValueExpr left;
  BinaryOp binaryOp;
  ValueExpr right;
} BinaryExpr;



int evalValueExpr(ValueExpr vexpr) {
  return vexpr.value;
}

int evalUnaryExpr(UnaryExpr uexpr) {
  int operandValue = evalValueExpr(uexpr.operand);
  switch (uexpr.unaryOp) {
  case UPLUS:
    return operandValue;
  case UMINUS:
    return -operandValue;
  default:
    exit(1);
    break;
  }
}

int evalBinaryExpr(BinaryExpr bexpr) {
  int leftValue = evalValueExpr(bexpr.left);
  int rightValue = evalValueExpr(bexpr.right);
  switch (bexpr.binaryOp) {
  case BPLUS:
    return leftValue + rightValue;
  case BMINUS:
    return leftValue - rightValue;
  case BTIMES:
    return leftValue * rightValue;
  case BDIV:
    return leftValue / rightValue;
  default:
    exit(1);
    break;
  }
}


int main(int argc, char **argv) {
  
  ValueExpr ve1 = { 1 };
  UnaryExpr ue1 = { UPLUS, { 1 } };
  BinaryExpr be1 = { { 2 }, BTIMES, { 3 } };

  printf("ve1 value is %d\n", evalValueExpr(ve1));
  printf("ue1 value is %d\n", evalUnaryExpr(ue1));
  printf("be1 value is %d\n", evalBinaryExpr(be1));
  
  
  return 0;
}
