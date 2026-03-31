#include <mulle-objc-runtime/mulle-objc-runtime.h>

// methods in @protocolclass default to @optional — no @optional keyword needed
@protocolclass Foo

- (void)doSomething;

@end

@protocolimplementation Foo
- (void)doSomething {}
@end

@interface MyClass : Foo < Foo>
@end

@implementation MyClass
// doSomething not implemented — should NOT warn with -Wprotocol
@end

int main(void) { return 0; }
