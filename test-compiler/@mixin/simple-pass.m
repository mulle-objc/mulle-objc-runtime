#include <mulle-objc-runtime/mulle-objc-runtime.h>

@mixin Foo;

@mixin Foo
@optional
- (void)doSomething;
@end

@implementation Foo
- (void)doSomething {}
@end

int main(void) { return 0; }
