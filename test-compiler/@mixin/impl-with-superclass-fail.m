#include <mulle-objc-runtime/mulle-objc-runtime.h>

// @implementation of mixin with superclass is an error
@interface Root
@end

@mixin Foo
- (void) doSomething;
@end

@implementation Foo : Root
- (void) doSomething {}
@end

int main(void) { return 0; }
