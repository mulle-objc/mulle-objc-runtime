#include <mulle-objc-runtime/mulle-objc-runtime.h>
// RHS method not found
@interface Foo
- (void)doSomething;
@end

@implementation Foo
@method_implementation -doSomething = -nonExistentMethod;
@end
