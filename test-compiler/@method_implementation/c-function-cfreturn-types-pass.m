#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

// C functions with various primitive return types matching ObjC declarations.
// Tests the hasSameUnqualifiedType arm of the return-type check.

// void return, zero-arg: no _param slot needed
static void   void_zero_arg(id self, SEL _cmd) {}

// int return matching ObjC int declaration
static int    int_one_arg(id self, SEL _cmd, void *_param) { return 0; }

// float return matching ObjC float declaration
static float  float_one_arg(id self, SEL _cmd, void *_param) { return 0.0f; }

// char * return: accepted by isAnyPointerType regardless of LHS
static char * charptr_one_arg(id self, SEL _cmd, void *_param) { return NULL; }

// struct * return: accepted by isAnyPointerType
struct PairData { int x; int y; };
static struct PairData *structptr_one_arg(id self, SEL _cmd, void *_param) { return NULL; }

// void * return with two ObjC args: still (id, SEL, void *)
static void * two_args(id self, SEL _cmd, void *_param) { return NULL; }

@interface Foo
- (void)          doVoidZeroArg;
- (int)           doInt:(void *)p;
- (float)         doFloat:(void *)p;
- (char *)        doCharPtr:(void *)p;
- (struct PairData *) doStructPtr:(void *)p;
- (void *)        doTwoArgs:(int)a with:(int)b;
@end

@implementation Foo
@method_implementation -doVoidZeroArg    = void_zero_arg;
@method_implementation -doInt:           = int_one_arg;
@method_implementation -doFloat:         = float_one_arg;
@method_implementation -doCharPtr:       = charptr_one_arg;
@method_implementation -doStructPtr:     = structptr_one_arg;
@method_implementation -doTwoArgs:with:  = two_args;
@end

int main(void) { return 0; }
