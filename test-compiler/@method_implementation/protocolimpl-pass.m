#include <mulle-objc-runtime/mulle-objc-runtime.h>
// @protocol_implementation with @method_implementation
@protocol Foo
- (void)doSomething;
- (void)doSomethingElse;
@end

@protocol_implementation Foo
- (void)doSomething {}
@method_implementation -doSomethingElse = -doSomething;
@end

int main(void) { return 0; }
