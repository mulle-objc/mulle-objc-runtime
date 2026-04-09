#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

// simplest case: alias void no-arg method to another
@interface Foo
- (void)doSomething;
- (void)doSomethingElse;
@end

@implementation Foo
- (void)doSomething {}
@method_implementation -doSomethingElse = -doSomething;
@end

int main(void) { return 0; }
