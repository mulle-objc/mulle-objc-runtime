#include <mulle-objc-runtime/mulle-objc-runtime.h>
// LHS selector not declared in @interface — should error
@interface Foo
- (void)doSomething;
@end

@implementation Foo
- (void)doSomething {}
@method_implementation -undeclaredMethod = -doSomething;
@end
