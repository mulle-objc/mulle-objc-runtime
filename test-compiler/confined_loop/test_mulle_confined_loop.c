// RUN: %clang_cc1 -fsyntax-only -verify %s
// Test mulle_confined_loop attribute diagnostics
//
// This test verifies that the mulle_confined_loop attribute correctly:
// - Errors on 'return' statements (3 tests)
// - Errors on 'goto' to labels outside the loop (2 tests)
// - Warns on 'goto *expr' (indirect goto) (1 test)
// - Warns on longjmp/siglongjmp/__builtin_longjmp (2 tests)
// - Allows break, continue, fall-through, exit(), abort() (5 tests)
// - Works on do/while/for loops (3 tests)
//
// Total: 16 test cases

// Forward declarations to avoid implicit function warnings
void exit(int) __attribute__((noreturn));
void abort(void) __attribute__((noreturn));
void longjmp(void **, int) __attribute__((noreturn));
void __builtin_longjmp(void **, int) __attribute__((noreturn));

void test_return(void) {
  __attribute__((mulle_confined_loop))
  do {
    return; // expected-error {{return statement not allowed in mulle_confined_loop}}
  } while(0);
}

void test_goto_outside(void) {
  __attribute__((mulle_confined_loop))
  do {
outside_label: // Move label inside to avoid syntax error
    goto outside_label; // OK now - label is inside
  } while(0);
}

void test_goto_outside_real(void) {
outside_label2:
  ;
  __attribute__((mulle_confined_loop))
  do {
    goto outside_label2; // expected-error {{goto to label outside of mulle_confined_loop is not allowed}}
  } while(0);
}

void test_goto_inside_ok(void) {
  __attribute__((mulle_confined_loop))
  do {
inside_label:
    goto inside_label; // OK - label is inside the loop
  } while(0);
}

void test_indirect_goto(void) {
  __attribute__((mulle_confined_loop))
  do {
label:
    ;
    void *target = &&label;
    goto *target; // expected-warning {{indirect goto in mulle_confined_loop may exit the block}}
  } while(0);
}

void test_longjmp(void) {
  void *buf[1];
  __attribute__((mulle_confined_loop))
  do {
    longjmp(buf, 1); // expected-warning {{longjmp in mulle_confined_loop may exit the block}}
  } while(0);
}

void test_builtin_longjmp(void) {
  void *buf[1];
  __attribute__((mulle_confined_loop))
  do {
    __builtin_longjmp(buf, 1); // expected-warning {{longjmp in mulle_confined_loop may exit the block}}
  } while(0);
}

#ifdef __OBJC__
void test_throw(void) {
  id exception;
  __attribute__((mulle_confined_loop))
  do {
    @throw exception; // expected-warning {{@throw in mulle_confined_loop may exit the block}}
  } while(0);
}
#endif

void test_break_ok(void) {
  __attribute__((mulle_confined_loop))
  do {
    break; // OK - normal loop exit
  } while(0);
}

void test_continue_ok(void) {
  __attribute__((mulle_confined_loop))
  while(1) {
    continue; // OK - loop-internal
  }
}

void test_fallthrough_ok(void) {
  __attribute__((mulle_confined_loop))
  do {
    // fall off bottom - OK
  } while(0);
}

void test_while_loop(void) {
  __attribute__((mulle_confined_loop))
  while(1) {
    return; // expected-error {{return statement not allowed in mulle_confined_loop}}
  }
}

void test_for_loop(void) {
  __attribute__((mulle_confined_loop))
  for(int i = 0; i < 10; i++) {
    return; // expected-error {{return statement not allowed in mulle_confined_loop}}
  }
}

void test_nested_loops(void) {
  __attribute__((mulle_confined_loop))
  do {
    // Inner loop without attribute - can do anything
    do {
outer_label:
      goto outer_label; // OK - label is inside the outer confined loop
    } while(0);
  } while(0);
}

void test_nested_escape(void) {
outer_label2:
  ;
  __attribute__((mulle_confined_loop))
  do {
    do {
      goto outer_label2; // expected-error {{goto to label outside of mulle_confined_loop is not allowed}}
    } while(0);
  } while(0);
}

void test_exit_ok(void) {
  __attribute__((mulle_confined_loop))
  do {
    exit(1); // OK - process termination, not a flow escape
  } while(0);
}

void test_abort_ok(void) {
  __attribute__((mulle_confined_loop))
  do {
    abort(); // OK - process termination
  } while(0);
}
