#include <mulle-objc-runtime/mulle-objc-runtime.h>

// explicit @required in @protocolclass still works
@protocolclass Foo

@required
- (void)mustImplement;

@end

@protocolimplementation Foo
- (void)mustImplement {}
@end

// adopting class that does NOT inherit from Foo — must implement @required
@interface MyClass < Foo>
@end

@implementation MyClass
// mustImplement not implemented — should warn
@end

int main(void) { return 0; }
