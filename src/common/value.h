#ifndef VALUE_H
#define VALUE_H

#include "types.h"

#include <stddef.h>
#include <stdint.h>

typedef struct {
  ValueType vtype;
  size_t size;
  uint64_t *octabytes;
} Value;

Value *makeValue(ValueType vtype, size_t size, void *values);
void make_ui64ValueAt(Value* valuePtr, uint64_t x);
Value *make_ui64Value(uint64_t x);
Value *makeStringValue(char *str);
void freeValue(Value *value);

#endif