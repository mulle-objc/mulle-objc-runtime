#include <mulle-objc-runtime/mulle-objc-runtime.h>
// outside @implementation — should error
@interface Foo
- (void)doSomething;
@end

@method_implementation -doSomething = -doSomething;
