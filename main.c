#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

typedef enum {
  cintValueType, // this is probably bad (??)
  ui64ValueType,
  StringValueType,
  CustomValueType,
} ValueType;

typedef struct {
  ValueType vtype;
  size_t size;
  uint64_t *octabytes;
} Value;

typedef enum {
  NullExpr,
  ValueExpr,
  BinAddExpr,
  ExprListExpr,
  CallExpr,
  StatementExpr,
  BlockExpr,
  FunctionDeclExpr,
  TypeExpr,
  VariableDeclExpr,
  IdentifierExpr,
  GroupingExpr,
} ExprType;

typedef struct Expr Expr;

struct Expr {
  ExprType etype;
  size_t subexpr_count;
  void *subexprs;
};

// typedef struct {
//   Expr *expr;
// } Statement;

typedef struct {
  size_t size;
  size_t next;
  void *arr;
} DynamicArray;

typedef struct {
  size_t expr_count;
  size_t expr_space;
  Expr **exprs;
} Scope;

typedef struct {
  size_t size;
  size_t next;  //maybe "used" instead of "next" ?
  char *buf;
} Buffer;


void scopeToBuffer(Buffer *buffer, Scope *scope);

void freeScope(Scope *scope);

void evalTypeExpr(Buffer *buffer, Expr *expr);




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
  Value *valuePtr = makeValue(ui64ValueType, 1, NULL);
  make_ui64ValueAt(valuePtr, x);
  return valuePtr;
}

Value *makeStringValue(char *str) {
  // the literal whole reason I wrote makeValue before was so that I could
  // use one function for making different types of values...
  // maybe I should make this function use that one instead, but for now this
  // is better
  uint64_t *strOctabytesPtr = malloc((strlen(str) + 1) * sizeof (uint64_t));
  for (int i = 0; i < strlen(str); ++i) strOctabytesPtr[i] = str[i];
  strOctabytesPtr[strlen(str)] = '\0';
  Value *valuePtr = malloc(sizeof (Value));
  valuePtr->vtype = StringValueType;
  valuePtr->size = strlen(str) + 1;
  valuePtr->octabytes = strOctabytesPtr;
  return valuePtr;
  //TODO: check malloc fail
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

Expr *makeExprListExpr(size_t exprs, Expr **exprPtrs) {
  return makeExpr(ExprListExpr, exprs, exprPtrs);
}

// Expr *makeCallExpr(char *name, Expr **args) {
//   Expr **subexprs = malloc(sizeof (char *) + sizeof (Expr **));
//   subexprs[0] = (void *)name; // feels like I am kind of cheating the type system.
//   subexprs[1] = (void *)args;
//   return makeExpr(CallExpr, 2, subexprs);
// }

Expr *makeCallExpr(char *name, Expr *exprListExprPtr) {
  // I'm not sure if I should really be passing a char * here or a Value...
  assert(exprListExprPtr->etype == ExprListExpr);
  Expr **subexprs = malloc(sizeof (char *) + sizeof (Expr **));
  subexprs[0] = (void *)name;
  subexprs[1] = exprListExprPtr;
  return makeExpr(CallExpr, 2, subexprs);
}

Expr *makeStatementExpr(Expr *expr) {
  return makeExpr(StatementExpr, 1, expr);
}

Expr *makeBlockExpr(Scope *scope) {
  return makeExpr(BlockExpr, 1, scope);
}

Expr *makeFunctionDeclExpr(char *name, Expr *type, Expr *args, Expr *body) {
  assert(type->etype == TypeExpr && args->etype == ExprListExpr &&
    body->etype == BlockExpr);
  Expr **subexprs = malloc(sizeof (char *) + sizeof (Expr **) * 3);
  subexprs[0] = (void *)name;
  subexprs[1] = type;
  subexprs[2] = args;
  subexprs[3] = body;
  return makeExpr(FunctionDeclExpr, 4, subexprs);
}

Expr *makeTypeExpr(ValueType vType, int64_t customTypeID) {
  Expr **subexprs = malloc(2 * sizeof (void *));
  subexprs[0] = (void *)vType;
  subexprs[1] = (void *)customTypeID;
  return makeExpr(TypeExpr, 2, subexprs);
}

Expr *makeVariableDeclExpr(Expr *type, char *name) {
  Expr **subexprs = malloc(sizeof (Expr *) + sizeof (char *));
  subexprs[0] = type;
  subexprs[1] = (void *)name;
  return makeExpr(VariableDeclExpr, 2, subexprs);
}

Expr *makeIdentifierExpr(char *lexeme) {
  return makeExpr(IdentifierExpr, 1, lexeme);
}

Expr *makeGroupingExpr(Expr *expr) {
  return makeExpr(GroupingExpr, 1, expr);
}

/*
 * recursively free expression, subexprs, and values in subexprs.
 * works now because I'm never sharing resources across expressions.
 * will break things later if I start sharing exprs...
 */
void freeExpr(Expr *exprPtr) {
  switch (exprPtr->etype) {
  case NullExpr:
    break;
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
  case ExprListExpr:
    for (int i = 0; i < exprPtr->subexpr_count; ++i) {
      freeExpr(((Expr **)exprPtr->subexprs)[i]);
    }
    free(exprPtr->subexprs);
    break;
  case CallExpr:
    freeExpr(((Expr **)exprPtr->subexprs)[1]);
    free(exprPtr->subexprs);
    break;
  case StatementExpr:
    freeExpr(exprPtr->subexprs);
    break;
  case BlockExpr:
    freeScope(exprPtr->subexprs);
    break;
  case FunctionDeclExpr:
    //free(((char *)exprPtr->subexprs)[0]); // I will probably need this later
    freeExpr(((Expr **)exprPtr->subexprs)[1]);
    freeExpr(((Expr **)exprPtr->subexprs)[2]);
    freeExpr(((Expr **)exprPtr->subexprs)[3]);
    free(exprPtr->subexprs);
    break;
  case TypeExpr:
    free(exprPtr->subexprs);
    break;
  case VariableDeclExpr:
    freeExpr(((Expr **)exprPtr->subexprs)[0]);
    free(exprPtr->subexprs);
    break;
  case IdentifierExpr:
    // do nothing, for now because I'm storing identifier by lexeme
    // right now the only lexemes I'm using source code level strings
    // which are handled by main somehow
    // and later I'll just store them by position in a source file
    // so I should never have to deallocate them... i think.
    break;
  case GroupingExpr:
    freeExpr(exprPtr->subexprs);
    break;
  default:
    printf("ERROR: tried to free unknown Expr type %d\n", exprPtr->etype);
    exit(1);
  }
  
  //printf("freeing generic expression pointer\n");
  free(exprPtr);
}

// void freeValueExpr(Expr *exprPtr) {
//   freeValue(exprPtr->subexprs);
//   free(exprPtr);
// }

void breaker() { }





void initializeBuffer(Buffer *buffer) {
  for (size_t i = 0; i < buffer->size; ++i) {
    buffer->buf[i] = 0;
  }
}

Buffer *makeBuffer(size_t size) {
  //TODO: error handling on malloc
  Buffer *buffer = malloc(sizeof (Buffer));
  char *buf = malloc((size > 0 ? size : 1) * sizeof (char));
  buffer->size = (size > 0 ? size : 1);
  buffer->next = 0;
  buffer->buf = buf;
  initializeBuffer(buffer);
  return buffer;
}

void freeBuffer(Buffer *buffer) {
  free(buffer->buf);
  free(buffer);
}

/* Increase buffer size to new_size and '\0' initialize new bytes at buffer end
 */
void BufferIncreaseSize(Buffer *buffer, size_t new_size) {
  if (new_size < buffer->size) assert(0 && "TODO: handle passing smaller size error");
  buffer->size = new_size;
  char *result = realloc(buffer->buf, buffer->size);
  if (result == NULL) assert(0 && "TODO: handle realloc error");
  buffer->buf = result;
  for (int i = buffer->next; i < buffer->size; ++i) buffer->buf[i] = '\0';
}

void BufferDoubleSize(Buffer *buffer) {
  BufferIncreaseSize(buffer, buffer->size * 2);
}

// this function should probably make buffer bigger
bool BufferWriteChar(Buffer *buffer, char c) {
  if (buffer->next == buffer->size - 1) { // writing to last char in buf
    BufferDoubleSize(buffer); 
  }

  buffer->buf[buffer->next++] = c;
  return true;
}

/* This function is dangerous in the case where the passed c_str is improperly
 * formatted without a null terminator. This shouldn't happen because I'm only
 * using it with string literals for now... */
void BufferWriteString(Buffer *buffer, char *c_str) {
  while (*c_str != '\0')
    BufferWriteChar(buffer, *c_str++);
}

void BufferNewline(Buffer *buffer) {
  BufferWriteChar(buffer, '\n');
}

void eval_ui64Value(Buffer *buffer, Value *value) {
  if (value->vtype != ui64ValueType || value->size != 1) {
    assert(0 && "Unexpected value type");
  }

  Buffer *tmp = makeBuffer(0);

  int rep_len = snprintf(tmp->buf,
    tmp->size, "%d", value->octabytes[0]);

  if (rep_len + 1 > tmp->size) {
    BufferIncreaseSize(tmp, rep_len + 1);
    rep_len = snprintf(tmp->buf, tmp->size, "%d", value->octabytes[0]);
    assert(tmp->size >= rep_len + 1);
  }

  BufferWriteString(buffer, tmp->buf);

  freeBuffer(tmp);
}

void evalStringValue (Buffer *buffer, Value *value) {
  for (int i = 0; i < value->size - 1; ++i) { // don't write null terminator
    BufferWriteChar(buffer, (char)value->octabytes[i]);
  }
}

void evalValue(Buffer *buffer, Value *value) {
  switch (value->vtype) {
    case ui64ValueType:
      eval_ui64Value(buffer, value);
      break;
    case StringValueType:
      evalStringValue(buffer, value);
      break;
    default:
      assert(0 && "Unexpected or unimplemented value type");
  }
}

void evalValueExpr(Buffer *, Expr *);
void evalBinAddExpr(Buffer *, Expr *);
void evalExpr(Buffer *, Expr *);


void evalValueExpr(Buffer *buffer, Expr *expr) {
  assert(expr->etype == ValueExpr);   //TODO: MAKE THIS BETTER
  evalValue(buffer, expr->subexprs);
}

void evalBinAddExpr(Buffer *buffer, Expr *expr) {
  assert(expr->etype == BinAddExpr);  //TODO: MAKE THIS BETTER
  evalExpr(buffer, ((Expr **)expr->subexprs)[0]);
  BufferWriteChar(buffer, '+');
  evalExpr(buffer, ((Expr **)expr->subexprs)[1]);
}

void evalCallExpr(Buffer *buffer, Expr *expr) {
  char *functionName = ((char **)expr->subexprs)[0];
  Expr *args = ((Expr **)expr->subexprs)[1];
  BufferWriteString(buffer, functionName);
  BufferWriteChar(buffer, '(');
  for (int i = 0; i < args->subexpr_count; ++i) {
    evalExpr(buffer, ((Expr **)args->subexprs)[i]);
    if (i + 1 < args->subexpr_count) {
      BufferWriteChar(buffer, ',');
      BufferWriteChar(buffer, ' ');
    }
  }
  BufferWriteChar(buffer, ')');
}

void evalStatementExpr(Buffer *buffer, Expr *expr) {
  evalExpr(buffer, expr->subexprs);
  BufferWriteChar(buffer, ';');
  BufferWriteChar(buffer, '\n');
}

void evalBlockExpr(Buffer *buffer, Expr *expr) {
  BufferWriteString(buffer, "{\n");
  scopeToBuffer(buffer, expr->subexprs);
  BufferWriteString(buffer, "}\n");
}

void evalFunctionDeclExpr(Buffer *buffer, Expr *expr) {
  char *name = ((char **)expr->subexprs)[0];
  Expr *type = ((Expr **)expr->subexprs)[1];
  Expr *args = ((Expr **)expr->subexprs)[2];
  Expr *body = ((Expr **)expr->subexprs)[3];
  evalTypeExpr(buffer, type);
  BufferWriteString(buffer, name);
  BufferWriteChar(buffer, '(');
  for (int i = 0; i < args->subexpr_count; ++i) {
    evalExpr(buffer, ((Expr **)args->subexprs)[i]);
    if (i + 1 < args->subexpr_count) {
      BufferWriteChar(buffer, ',');
      BufferWriteChar(buffer, ' ');
    }
  }
  BufferWriteChar(buffer, ')');
  evalExpr(buffer, body);
}

void evalTypeExpr(Buffer *buffer, Expr *expr) {
  switch ((int64_t)(((ValueType **)expr->subexprs)[0])) {
    case ui64ValueType:
      BufferWriteString(buffer, "uint64_t ");
      break;
    case StringValueType:
      BufferWriteString(buffer, "(char *) ");
      break;
    case CustomValueType:
      assert(0 && "TODO: implement custom types");
      break;
    default:
      assert(0 && "Unexpected or unimplemented value type");
  }
}

void evalVariableDeclExpr(Buffer *buffer, Expr *expr) {
  Expr *type = ((Expr **)expr->subexprs)[0];
  char *name = ((char **)expr->subexprs)[1];
  evalTypeExpr(buffer, type);
  BufferWriteString(buffer, name);
}

void evalIdentifierExpr(Buffer *buffer, Expr *expr) {
  BufferWriteString(buffer, (char *)expr->subexprs); // will this work ??
}

void evalGroupingExpr(Buffer *buffer, Expr *expr) {
  BufferWriteChar(buffer, '(');
  evalExpr(buffer, expr->subexprs);
  BufferWriteChar(buffer, ')');
}

void evalExpr(Buffer *buffer, Expr *expr) {
  switch(expr->etype) {
    case ValueExpr:
      evalValueExpr(buffer, expr);
      return;
    case BinAddExpr:
      evalBinAddExpr(buffer, expr);
      return;
    case CallExpr:
      evalCallExpr(buffer, expr);
      return;
    case StatementExpr:
      evalStatementExpr(buffer, expr);
      return;
    case BlockExpr:
      evalBlockExpr(buffer, expr);
      return;
    case FunctionDeclExpr:
      evalFunctionDeclExpr(buffer, expr);
      return;
    case TypeExpr:
      evalTypeExpr(buffer, expr);
      return;
    case VariableDeclExpr:
      evalVariableDeclExpr(buffer, expr);
      return;
    case IdentifierExpr:
      evalIdentifierExpr(buffer, expr);
      return;
    case GroupingExpr:
      evalGroupingExpr(buffer, expr);
      return;
    default:
      assert(0 && "Error: Expr has unknown etype");
  }
}

bool run_tests() {
  //evalValue test
  Buffer *b = makeBuffer(0);
  Value *v = make_ui64Value(12345678);
  evalValue(b, v);
  char *expected = "12345678\0";
  assert(b->size > strlen(expected)); //strlen(expected) is 8. expect b->size to be at least 8 + 1 because it has a \0 terminator
  for (int i = 0; i <= strlen(expected); ++i) {
    assert(b->buf[i] == expected[i]);
  }
  freeBuffer(b);
  freeValue(v);

  Value *strValue = makeStringValue("Hello, World");
  b = makeBuffer(0);
  evalValue(b, strValue);

  BufferNewline(b);
  expected = "Hello, World\n";
  for (int i = 0; i <= strlen(expected); ++i) {
    assert(b->buf[i] == expected[i]);
  }
  //printf(b->buf);
  freeValue(strValue);
  freeBuffer(b);

  return true;
}


void initializeScope(Scope *scope) {
  for (size_t i = 0; i < scope->expr_space; ++i) {
    // scope->exprs[i].etype = NullExpr;
    // scope->exprs[i].subexpr_count = 0;
    // scope->exprs[i].subexprs = NULL;
    scope->exprs[i] = NULL;
  }
}

Scope *makeScope(size_t size) {
  Scope *scope = malloc(sizeof (Scope));
  scope->expr_count = 0;
  scope->expr_space = size;
  scope->exprs = malloc(size * sizeof (Expr));
  return scope;
}

void freeScope(Scope *scope) {
  for (size_t i = 0; i < scope->expr_count; ++i) {
    freeExpr(scope->exprs[i]);
  }
  free(scope->exprs);
  free(scope);
}

void scopeIncreaseSize(Scope *scope, size_t new_size) {
  if (new_size < scope->expr_space) assert(0 && "TODO: handle passing smaller size error");
  scope->expr_space = new_size;
  Expr **result = realloc(scope->exprs, scope->expr_space);
  if (result == NULL) assert(0 && "TODO: handle realloc error");
  scope->exprs = result;
  initializeScope(scope);
}

void scopeDoubleSize(Scope *scope) {
  scopeIncreaseSize(scope, scope->expr_space * 2);
}

void scopeAddExpr(Scope *scope, Expr *expr) {
  if (scope->expr_count == scope->expr_space) {
    scopeDoubleSize(scope);
  }
  scope->exprs[scope->expr_count++] = expr;
}

void scopeToBuffer(Buffer *buffer, Scope *scope) {
  for (size_t i = 0; i < scope->expr_count; ++i) {
    evalExpr(buffer, scope->exprs[i]);
  }
}

void evalMainScope(Buffer *buffer, Scope *scope) {
  BufferWriteString(buffer, "int main() {\n");
  scopeToBuffer(buffer, scope);
  BufferWriteString(buffer, "return 0;\n}\n");
}



int main(int argc, char **argv) {

  assert(run_tests());

  Buffer *output = makeBuffer(1024);

  Value *value1 = make_ui64Value(12345);
  Expr *statement1 = makeExpr(StatementExpr, 1, makeValueExpr(value1));

  Expr *statement2 = makeExpr(StatementExpr, 1, makeBinAddExpr(
    makeValueExpr(make_ui64Value(700)),
    makeValueExpr(make_ui64Value(77))));

  Expr *statement3 = makeExpr(StatementExpr, 1, makeBinAddExpr(
    makeBinAddExpr(
      makeValueExpr(make_ui64Value(1000)),
      makeValueExpr(make_ui64Value(100))),
    makeBinAddExpr(
      makeValueExpr(make_ui64Value(10)),
      makeValueExpr(make_ui64Value(1)))));


  Expr *expr4 = makeValueExpr(makeStringValue("\"%d\\n\""));  
  Expr *expr5 = makeValueExpr(make_ui64Value(111));
  Expr **call_args = malloc(2 * sizeof (Expr *));
  call_args[0] = expr4;
  call_args[1] = expr5;
  Expr *statement4 = makeExpr(StatementExpr, 1,
    makeCallExpr("printf", makeExprListExpr(2, call_args)));


  Scope *mainScope = makeScope(128);
  scopeAddExpr(mainScope, statement1);
  scopeAddExpr(mainScope, statement2);
  scopeAddExpr(mainScope, statement3);
  scopeAddExpr(mainScope, statement4);


  Expr *statement5 = makeExpr(StatementExpr, 1, makeBinAddExpr(
    makeValueExpr(make_ui64Value(11)),
    makeValueExpr(make_ui64Value(22))));
  Expr *statement6 = makeExpr(StatementExpr, 1, makeBinAddExpr(
    makeValueExpr(make_ui64Value(33)),
    makeValueExpr(make_ui64Value(44))));
  Scope *scope1 = makeScope(128);
  scopeAddExpr(scope1, statement5);
  scopeAddExpr(scope1, statement6);
  Expr *expr6 = makeExpr(BlockExpr, 1, scope1);
  scopeAddExpr(mainScope, expr6);
  



  evalMainScope(output, mainScope);

  printf("scope expr count %d\n", mainScope->expr_count);
  freeScope(mainScope);

  puts(output->buf);    //I can't use printf to display output because printf
  // tries to format the %d inside the string but I want raw output

  freeBuffer(output);

  Expr *type1 = makeTypeExpr(ui64ValueType, 0);
  freeExpr(type1);

  Expr *varDecl1 = makeVariableDeclExpr(makeTypeExpr(ui64ValueType, 0), "x");
  Expr *varDecl2 = makeVariableDeclExpr(makeTypeExpr(ui64ValueType, 0), "y");
  Expr **varDecls = malloc(2 * sizeof (Expr *));
  varDecls[0] = varDecl1;
  varDecls[1] = varDecl2;
  Expr *args1 = makeExpr(ExprListExpr, 2, varDecls);
  freeExpr(args1);

  Buffer *output2 = makeBuffer(1024);
  
  char *name = "add";
  Expr *type = makeTypeExpr(ui64ValueType, 0);
  //Expr *args = makeExprListExpr(2, (Expr *[]){makeVariableDeclExpr(makeTypeExpr(ui64ValueType, 0), "x"), makeVariableDeclExpr(makeTypeExpr(ui64ValueType, 0), "y")} );
  Expr *varX = makeVariableDeclExpr(makeTypeExpr(ui64ValueType, 0), "x");
  Expr *varY = makeVariableDeclExpr(makeTypeExpr(ui64ValueType, 0), "y");
  Expr **argsArray = malloc(2 * sizeof (Expr *));
  argsArray[0] = varX;
  argsArray[1] = varY;
  Expr *args = makeExpr(ExprListExpr, 2, argsArray);
  Expr *body = makeExpr(BlockExpr, 1, makeScope(128));
  Expr *functionDecl = makeFunctionDeclExpr(name, type, args, body);
  evalFunctionDeclExpr(output2, functionDecl);
  freeExpr(functionDecl);
  puts(output2->buf);
  freeBuffer(output2);

  Buffer *output3 = makeBuffer(1024);

  Expr *idExpr = makeIdentifierExpr("x");
  Expr *stmtExpr = makeStatementExpr(idExpr);
  Expr *body2 = makeExpr(BlockExpr, 1, makeScope(128));
  scopeAddExpr(body2->subexprs, stmtExpr);
  evalBlockExpr(output3, body2);
  freeExpr(body2);
  puts(output3->buf);
  freeBuffer(output3);


  {
    Buffer *output = makeBuffer(1024);
    Expr *retType = makeTypeExpr(ui64ValueType, 0);
    Expr *arg1Type = makeTypeExpr(ui64ValueType, 0);
    Expr *arg2Type = makeTypeExpr(ui64ValueType, 0);
    Expr *arg1 = makeVariableDeclExpr(arg1Type, "x");
    Expr *arg2 = makeVariableDeclExpr(arg2Type, "y");
    Expr **argArray = malloc(2 * sizeof (Expr *));
    argArray[0] = arg1;
    argArray[1] = arg2;
    Expr *args = makeExpr(ExprListExpr, 2, argArray);
    Scope *bodyScope = makeScope(128);
    Expr *stmt1 = makeStatementExpr(
      makeBinAddExpr(
        makeIdentifierExpr("x"), 
        makeIdentifierExpr("y")));
    scopeAddExpr(bodyScope, stmt1);

    Expr *addXY1 = makeGroupingExpr(
      makeBinAddExpr(
        makeIdentifierExpr("x"), 
        makeIdentifierExpr("y")));
    Expr *addXY2 = makeGroupingExpr(
      makeBinAddExpr(
        makeIdentifierExpr("x"), 
        makeIdentifierExpr("y")));
    Expr *stmt2 = makeStatementExpr(  
      makeBinAddExpr(
        addXY1,
        addXY2));
    scopeAddExpr(bodyScope, stmt2);
    // TODO: implement return statement
    Expr *body = makeExpr(BlockExpr, 1, bodyScope);
    Expr *functionDecl = makeFunctionDeclExpr("add", retType, args, body);
    evalFunctionDeclExpr(output, functionDecl);
    puts(output->buf);
    freeExpr(functionDecl);
    freeBuffer(output);
  }
  
  
  return 0;
}
