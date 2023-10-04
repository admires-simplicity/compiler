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
  BinAddExpr,
} ExprType;


typedef struct Expr Expr;

struct Expr {
  ExprType etype;
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
  valuePtr->size = 1;
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
  subexprs[0] = subexpr0;
  subexprs[1] = subexpr1;
  return makeExpr(BinAddExpr, 2, subexprs);
}


/*
 * recursively free expression, subexprs, and values in subexprs.
 * works now because I'm never sharing resources across expressions.
 * will break things later if I start sharing exprs...
 */
void freeExpr(Expr *exprPtr) {
  switch (exprPtr->etype) {
  case ValueExpr:
    //printf("freeing value expression\n");
    freeValue(exprPtr->subexprs);
    break;
  case BinAddExpr:
    //printf("freeing bin add expression\n");
    for (int i = 0; i < exprPtr->subexpr_count; ++i) {
      //printf("freeing bin add subexpr %d\n", i);
      freeExpr(((Expr **)exprPtr->subexprs)[i]);
    }
    //printf("freeing subexpression dynamic array\n");
    free(exprPtr->subexprs);
    break;
  default:
    //printf("ERROR: tried to free unknown Expr type\n");
    exit(1);
  }
  
  //printf("freeing generic expression pointer\n");
  free(exprPtr);
}

void freeValueExpr(Expr *exprPtr) {
  freeValue(exprPtr->subexprs);
  free(exprPtr);
}

void breaker() { }

typedef struct {
  size_t size;
  size_t next;  //maybe "used" instead of "next" ?
  char *buf;
} Buffer;

void initializeBuffer(Buffer *buffer) {
  for (size_t i = 0; i < buffer->size; ++i) {
    buffer->buf[i] = 0;
  }
}

Buffer *makeBuffer(size_t size) {
  //TODO: error handling on malloc
  Buffer *buffer = malloc(sizeof (Buffer));
  char *buf = malloc(size * sizeof (char));
  buffer->size = size;
  buffer->next = 0;
  buffer->buf = buf;
  initializeBuffer(buffer);
  return buffer;
}

void freeBuffer(Buffer *buffer) {
  free(buffer->buf);
  free(buffer);
}

// bool writeBuffer(Buffer *dst, Buffer *src) {
//   if (dst->next + src->next + 1 > dst->size) { // require an extra '\0' at buf end
//     return false; // TODO: make buffer bigger
//   }
//   for (int i = 0; i < src->size; ++i) {
//     dst->buf[dst->next++] = src->buf[i];
//   }
// }

bool BufferWriteChar(Buffer *buffer, char c) {
  if (buffer->next == buffer->size) return false;

  buffer->buf[buffer->next++] = c;
  return true;
}

void evalValue(Buffer *buffer, Value *value) {
  if (value->vtype != ui64Value || value->size != 1) {
    assert(0 && "Unexpected\\unimplemented value type");
  }

  int to_write = snprintf(buffer->buf + buffer->next,
    buffer->size - buffer->next - 1, "%d", value->octabytes[0]);

  if (to_write > buffer->size - buffer->next - 1) {
    assert(0 && "TODO: handle error when int to write is larger than buffer");
  }

  buffer->next += to_write;
}

int main(int argc, char **argv) {
  Buffer *output = makeBuffer(1024);
  // BufferWriteChar(output, '1');
  // BufferWriteChar(output, '2');
  // BufferWriteChar(output, '3');
  // BufferWriteChar(output, '\n');
  // BufferWriteChar(output, '4');
  // BufferWriteChar(output, '5');
  // BufferWriteChar(output, '6');
  // BufferWriteChar(output, '\n');

  Value *value1 = make_ui64Value(12345);
  Expr *expr1 = makeExpr(ValueExpr, 1, value1);

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

  


  evalValue(output, value1);
  BufferWriteChar(output, '\n');



  printf("free expr1\n");
  freeExpr(expr1);
  printf("\n");

  printf("free expr2\n");
  freeExpr(expr2);
  printf("\n");

  printf("free expr3\n");
  freeExpr(expr3);
  printf("\n");

  breaker();

  printf(output->buf);

  freeBuffer(output);
  
  return 0;
}
