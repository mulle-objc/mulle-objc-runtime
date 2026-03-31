#include <mulle-objc-runtime/mulle-objc-runtime.h>
// C function not found
@interface Foo
- (void)doSomething;
@end

@implementation Foo
@method_implementation -doSomething = nonExistentCFunction;
@end
