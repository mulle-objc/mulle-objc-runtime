#include <mulle-objc-runtime/mulle-objc-runtime.h>
// @protocolimplementation with @method_implementation
@protocol Foo
- (void)doSomething;
- (void)doSomethingElse;
@end

@protocolimplementation Foo
- (void)doSomething {}
@method_implementation -doSomethingElse = -doSomething;
@end

int main(void) { return 0; }
