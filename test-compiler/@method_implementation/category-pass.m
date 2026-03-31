#include <mulle-objc-runtime/mulle-objc-runtime.h>
// @method_implementation inside a category @implementation
@interface Foo
@end

@interface Foo (MyCategory)
- (void)doSomething;
- (void)doSomethingElse;
@end

@implementation Foo (MyCategory)
- (void)doSomething {}
@method_implementation -doSomethingElse = -doSomething;
@end

int main(void) { return 0; }
