#include <stdlib.h>
#include <string.h>

#include "value.h"

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