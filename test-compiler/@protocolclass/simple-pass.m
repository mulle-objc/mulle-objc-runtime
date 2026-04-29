#include <mulle-objc-runtime/mulle-objc-runtime.h>

@protocol_class Foo;

@protocol_interface Foo
@optional
- (void)doSomething;
@end

@protocol_implementation Foo
- (void)doSomething {}
@end

int main(void) { return 0; }
