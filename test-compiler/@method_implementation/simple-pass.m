#include <mulle-objc-runtime/mulle-objc-runtime.h>

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
