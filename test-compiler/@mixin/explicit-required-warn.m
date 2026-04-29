#include <mulle-objc-runtime/mulle-objc-runtime.h>

// explicit @required in @mixin still works
@mixin Foo

@required
- (void)mustImplement;

@end

@implementation Foo
- (void)mustImplement {}
@end

// adopting class that does NOT inherit from Foo — must implement @required
@interface MyClass < Foo>
@end

@implementation MyClass
// mustImplement not implemented — should warn
@end

int main(void) { return 0; }
