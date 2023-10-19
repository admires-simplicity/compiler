#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "../common/value.h"
#include "../common/expr.h"
#include "../common/scope.h"


#define BEGIN (printf("beginning %s\n", __func__))
#define END (printf("all tests passed in %s\n", __func__))

// void scope_test() {
//   BEGIN;
//   assert(something(1) == 2);
//   END;
// }

void value_test() {
  BEGIN;
  Value *value = make_ui64Value(1);
  assert(value->vtype == ui64ValueType);
  assert(value->size == 1);
  assert(value->octabytes[0] == 1);
  freeValue(value);

  value = makeStringValue("hello");
  assert(value->vtype == StringValueType);
  assert(value->size == strlen("hello") + 1);
  for (int i = 0; i < strlen("hello") + 1; ++i) {
    assert(value->octabytes[i] == "hello"[i]);
  }
  freeValue(value);
  END;
}

int main() {
  value_test();
  
  // scope_test();

}