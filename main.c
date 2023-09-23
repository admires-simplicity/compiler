#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

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
  BinExpr,
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

Value *make_ui64Value(uint64_t x) {
  Value *value = makeValue(ui64Value, 1, NULL);
  value->octabytes[0] = x;
  return value;
}

void freeValue(Value *value) {
  free(value->octabytes);
  free(value);
}

Expr *makeExpr(ExprType etype, size_t subexpr_count, void *subexprs) {
  Expr *expr = malloc(sizeof (Expr));
  expr->etype = etype;
  expr->subexpr_count = subexpr_count;
  if (subexpr_count == 0 || subexprs == NULL) {
    // malformed. can't have an expression with no subexprs.
    // TODO: implement better error handling
    exit(1);
  }

  expr->subexprs = subexprs;
  return expr;
}

int main(int argc, char **argv) {
  
  Value *value1 = make_ui64Value(12345);
  Expr *expr1 = makeExpr(ValueExpr, 1, value1);

  printf("value1->octabytes[0] == %d\n", value1->octabytes[0]);
  printf("((Value *)expr1->subexprs)->octabytes[0] == %d\n", ((Value *)expr1->subexprs)->octabytes[0]);
  
  freeValue(value1);
  
  return 0;
}
