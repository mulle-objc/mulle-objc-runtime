#include <mulle-objc-runtime/mulle-objc-runtime.h>
// @implementation of mixin with @method_implementation
@mixin Foo
- (void)doSomething;
- (void)doSomethingElse;
@end

@implementation Foo
- (void)doSomething {}
@method_implementation -doSomethingElse = -doSomething;
@end

int main(void) { return 0; }
