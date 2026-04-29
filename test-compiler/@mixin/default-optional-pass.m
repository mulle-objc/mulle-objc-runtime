#include <mulle-objc-runtime/mulle-objc-runtime.h>

// methods in @mixin default to @optional — no @optional keyword needed
@mixin Foo

- (void) doSomething;

@end

@implementation Foo
- (void) doSomething {}
@end

@interface MyClass < Foo>
@end

@implementation MyClass
// doSomething not implemented — should NOT warn with -Wprotocol
@end

int main( void)
{
   return 0;
}
