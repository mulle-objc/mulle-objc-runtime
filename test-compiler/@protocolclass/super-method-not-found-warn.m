#include <mulle-objc-runtime/mulle-objc-runtime.h>

@protocolclass Foo;

@protocolclass Foo
@optional
- (void)doSomething;
@end

@protocolimplementation Foo
- (void)doSomething {}
@end

@interface MyClass : Foo < Foo>
@end

@implementation MyClass
- (void)doSomething {
    [super nonExistent];
}
@end

int main(void) { return 0; }
