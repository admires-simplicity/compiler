#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* enum OpCode */

/* typedef struct Operator { */
/*   OpCode opcode; */
/* } */

/* typedef struct Expr { */
/*   Operator operator; */
/*   Operands operands; */
/* } */

typedef struct type1 {
  int x;
} type1;

typedef struct type2 {
  int x;
  int *y;
} type2;

typedef struct type3 {
  type1 t1;
  type2 t2;
} type3;

/* typedef struct function { */
  
/* } */

typedef enum {
  ADD, SUB, MUL, DIV
} Op;

typedef struct UnaryOp {
  Op op;
  int x;
} UnOp;

typedef struct BinaryOp {
  Op op;
  int l;
  int r;
} BinOp;

int binEval(BinOp binOp) {
  switch (binOp.op) {
  case ADD:
    return binOp.l + binOp.r;
  default:
    return 0;
  }
}




typedef enum {
  value,
  identifier,
  function,
  braces
} ExprType;

typedef enum {
  num,
  chr,
} ValueType; 

typedef struct {
  ValueType vtype;
  uint64_t qw;
} Value;

typedef struct {
  ExprType etype;
  uint64_t qw;
} Expr;


uint64_t eval(Expr expr) {
  switch (expr.etype) {
  case value:
    return expr.qw;
  case identifier:
    return expr.qw;
  case function:
    return expr.qw;
      
  }
}




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




/* typedef struct Expr { */
/*   Expr operator; */
/*   Expr *operands; */
/* } Expr; */

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

  /* Expr(Op['+'],  */

  type3 t3 = { { 0 }, { 1, NULL } };
  int tmp[] = { 2, 3, 4 };
  t3.t2.y = tmp;

  printf("t3.t1.x = %d\n", t3.t1.x);
  printf("t3.t2.x = %d\n", t3.t2.x);
  for (int i = 0; i < 3; ++i) {
    printf("t3.t2.y[%d] = %d\n", i, t3.t2.y[i]);
  }
  
  BinOp x = { ADD, 1, 2 };
  printf("%d\n", binEval(x));

  Value a = { chr, 'a' };
  printf("a.vtype: %d\ta.qw: %d\n", a.vtype, a.qw);

  ValueExpr ve1 = { 1 };
  UnaryExpr ue1 = { UPLUS, { 1 } };
  BinaryExpr be1 = { { 2 }, BTIMES, { 3 } };

  printf("ve1 value is %d\n", evalValueExpr(ve1));
  printf("ue1 value is %d\n", evalUnaryExpr(ue1));
  printf("be1 value is %d\n", evalBinaryExpr(be1));
  
  
  return 0;
}
