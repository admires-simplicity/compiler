#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

typedef enum {
  ui64Value,
} ValueType;

typedef struct {
  ValueType vtype;
  size_t size;
  uint64_t *octabytes;
} Value;

typedef enum {
  ValueExpr,
  /* BinExpr, */
  BinAddExpr,
} ExprType;

/* typedef enum { */
/*   ValueOp, */
/*   PlusOp, */
/* } Operator; */

typedef struct Expr Expr;

struct Expr {
  ExprType etype;
  /* Operator op; */
  /* size_t noperands; */
  /* void *operands; */
  size_t subexpr_count;
  void *subexprs;
};

/*
 * vtype -- type of value to make
 * size -- number of octabytes to allocate for *octabytes
 * *values -- ptr to values to set octabytes to
 * 
 * must be passed a valid vtype and valid size. *values is optional and may be
 * NULL.
 */
Value *makeValue(ValueType vtype, size_t size, void *values) {
  Value *value = malloc(sizeof (Value));
  value->vtype = vtype;
  if (size > 0 && values != NULL) {
    // allocate space for octabytes and copy from values
    value->octabytes = malloc(size * sizeof (uint64_t));
    if (value->octabytes == NULL) exit(1);
    memcpy(value->octabytes, values, size);

    // TODO: think about -- should this take a preallocated *values and just set value->octabytes to values, or should it copy values??
  }
  else if (size > 0 && values == NULL) {
    // allocate space but don't copy
    value->octabytes = malloc(size * sizeof(uint64_t));
    if (value->octabytes == NULL) exit(1);
  }
  else if (size == 0) {
    // don't allow to not allocate an octabytes ptr,
    // so that we can always free octabytes in freeValue.

    // TODO: implement better error handling
    exit(1);
  }
  return value;
}

void make_ui64ValueAt(Value* valuePtr, uint64_t x) {
  valuePtr->octabytes[0] = x;
}

Value *make_ui64Value(uint64_t x) {
  Value *valuePtr = makeValue(ui64Value, 1, NULL);
  make_ui64ValueAt(valuePtr, x);
  return valuePtr;
}

void freeValue(Value *value) {
  free(value->octabytes);
  free(value);
}

Expr *exprAlloc() {
  return malloc(sizeof (Expr));
}

void makeExprAt(Expr *exprPtr, ExprType etype, size_t subexpr_count, void *subexprs) {
  exprPtr->etype = etype;
  exprPtr->subexpr_count = subexpr_count;
  if (subexpr_count == 0 && subexprs != NULL) {
    // malformed.
    // TODO: implement better error handling

    printf("ERROR: passed subexpr_count == 0 and subexprs != NULL to makeExprAt\n");
    exit(1);
  }

  exprPtr->subexprs = subexprs;
}
		
Expr *makeExpr(ExprType etype, size_t subexpr_count, void *subexprs) {
  Expr *exprPtr = exprAlloc();
  makeExprAt(exprPtr, etype, subexpr_count, subexprs);
  return exprPtr;
}

void *makeValueExprAt(Expr *exprPtr, Value *valuePtr) {
  makeExprAt(exprPtr, ValueExpr, 1, valuePtr);
}

Expr *makeValueExpr(Value *valuePtr) {
  Expr *exprPtr = exprAlloc();
  makeValueExprAt(exprPtr, valuePtr);
  return exprPtr;
}

Expr *makeBinAddExpr(Expr *subexpr0, Expr *subexpr1) {
  Expr **subexprs = malloc(2 * sizeof (Expr *));
  
  // makeExprAt(expr2_subexprs + 0, ValueExpr, 1, make_ui64Value(500));
  // makeExprAt(expr2_subexprs + 1, ValueExpr, 1, make_ui64Value(40));
  subexprs[0] = subexpr0;
  subexprs[1] = subexpr1;
  return makeExpr(BinAddExpr, 2, subexprs);
}

// void freeSubexpr(Expr *exprPtr) {
//   switch (exprPtr->etype) {
//   case ValueExpr: 
//     freeValue(exprPtr->subexprs);
//     break;
//   case BinAddExpr:
//     assert(0 && "TODO");
//   }
// }

/*
 * recursively free expression, subexprs, and values in subexprs.
 * works now because I'm never sharing resources across expressions.
 * will break things later if I start sharing exprs...
 */
void freeExpr(Expr *exprPtr) {
  switch (exprPtr->etype) {
  case ValueExpr:
    printf("freeing value expression\n");
    freeValue(exprPtr->subexprs);
    break;
  case BinAddExpr:
    printf("freeing bin add expression\n");
    for (int i = 0; i < exprPtr->subexpr_count; ++i) {
      printf("freeing bin add subexpr %d\n", i);
      //freeSubexpr((Expr *)(exprPtr->subexprs) + i);
      freeExpr(((Expr **)exprPtr->subexprs)[i]);
    }
    printf("freeing subexpression dynamic array\n");
    free(exprPtr->subexprs);
    break;
  default:
    printf("ERROR: tried to free unknown Expr type\n");
    exit(1);
  }
  
  printf("freeing generic expression pointer\n");
  free(exprPtr);
}

void freeValueExpr(Expr *exprPtr) {
  freeValue(exprPtr->subexprs);
  free(exprPtr);
}

void breaker() { }


int main(int argc, char **argv) {
  
  Value *value1 = make_ui64Value(12345);
  Expr *expr1 = makeExpr(ValueExpr, 1, value1);

  // Expr *expr2_subexprs = malloc(2 * sizeof (Expr));
  // makeExprAt(expr2_subexprs + 0, ValueExpr, 1, make_ui64Value(500));
  // makeExprAt(expr2_subexprs + 1, ValueExpr, 1, make_ui64Value(40));
  // Expr *expr2 = makeExpr(BinAddExpr, 2, expr2_subexprs);

  // Value *expr2_subexprs = malloc(2 * sizeof (Value));
  // makeExprAt(expr2_subexprs + 0, ValueExpr, 1, make_ui64Value(500));
  // makeExprAt(expr2_subexprs + 1, ValueExpr, 1, make_ui64Value(40));
  // Expr *expr2 = makeExpr(BinAddExpr, 2, expr2_subexprs);


  // // THIS USED TO WORK BUT I BROKE IT INTENTIONALLY
  // Expr *expr2_subexprs = malloc(2 * sizeof (Expr));
  // makeExprAt(expr2_subexprs + 0, ValueExpr, 1, make_ui64Value(500));
  // makeExprAt(expr2_subexprs + 1, ValueExpr, 1, make_ui64Value(40));
  // Expr *expr2 = makeExpr(BinAddExpr, 2, expr2_subexprs);

  Expr *expr2 = makeBinAddExpr(
    makeValueExpr(make_ui64Value(700)),
    makeValueExpr(make_ui64Value(77)));

  Expr *expr3 = makeBinAddExpr(
    makeBinAddExpr(
      makeValueExpr(make_ui64Value(1000)),
      makeValueExpr(make_ui64Value(100))),
    makeBinAddExpr(
      makeValueExpr(make_ui64Value(10)),
      makeValueExpr(make_ui64Value(1))));

  breaker();


  // printf("value1->octabytes[0] == %d\n", value1->octabytes[0]);
  // printf("((Value *)expr1->subexprs)->octabytes[0] == %d\n", ((Value *)expr1->subexprs)->octabytes[0]);

  // printf("((Value *)expr2_subexprs->subexprs)->octabytes[0] == %d\n", ((Value *)expr2_subexprs->subexprs)->octabytes[0]);
  // printf("((Value *)expr2_subexprs->subexprs)->octabytes[0] == %d\n", ((Value *)(expr2_subexprs + 1)->subexprs)->octabytes[0]);

  // printf("%d\n",
	//  ((Value *)(expr2_subexprs + 0)->subexprs)->octabytes[0]
	//  +
	//  ((Value *)(expr2_subexprs + 1)->subexprs)->octabytes[0]);


 
  
  // /* freeValue(value1); */


  printf("free expr1\n");
  freeExpr(expr1);
  printf("\n");

  printf("free expr2\n");
  freeExpr(expr2);
  printf("\n");

  printf("free expr3\n");
  freeExpr(expr3);
  printf("\n");
  
  return 0;
}
