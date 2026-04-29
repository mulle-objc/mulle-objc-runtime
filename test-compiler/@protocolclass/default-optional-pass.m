#include <mulle-objc-runtime/mulle-objc-runtime.h>

// methods in @protocol_interface default to @optional — no @optional keyword needed
@protocol_interface Foo

- (void) doSomething;

@end

@protocol_implementation Foo
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
