#include <mulle-objc-runtime/mulle-objc-runtime.h>

// @interface subclassing a mixin is an error
@mixin Foo
- (void) doSomething;
@end

@implementation Foo
- (void) doSomething {}
@end

@interface Bar : Foo
@end

@implementation Bar
@end

int main(void) { return 0; }
