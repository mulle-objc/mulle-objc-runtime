#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

// C function returns int, but ObjC method declares BOOL return.
// int and BOOL (enum _MulleBool) are different types, so this must error.

static int wrong_return_type(id self, SEL _cmd, void *_param) { return 0; }

@interface Foo
- (BOOL) doSomething:(void *)p;
@end

@implementation Foo
@method_implementation -doSomething: = wrong_return_type;
@end
