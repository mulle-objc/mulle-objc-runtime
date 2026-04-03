// Test mulle_confined_return attribute
// Verifies that return statements with mulle_confined_return are allowed
// in mulle_confined_loop but only at the top level (not in nested loops)

void test_confined_return_ok(void) {
  __attribute__((mulle_confined_loop))
  do {
    __attribute__((mulle_confined_return))
    return;  // OK - confined return at top level
  } while(0);
}

void test_confined_return_in_nested_loop(void) {
  __attribute__((mulle_confined_loop))
  do {
    while(1) {
      __attribute__((mulle_confined_return))
      return;  // ERROR - confined return in nested loop
    }
  } while(0);
}

void test_regular_return_still_errors(void) {
  __attribute__((mulle_confined_loop))
  do {
    return;  // ERROR - regular return without attribute
  } while(0);
}

void test_confined_return_with_break(void) {
  __attribute__((mulle_confined_loop))
  do {
    if (1) {
      __attribute__((mulle_confined_return))
      return;  // OK - in if block but not in nested loop
    }
    break;  // OK
  } while(0);
}
