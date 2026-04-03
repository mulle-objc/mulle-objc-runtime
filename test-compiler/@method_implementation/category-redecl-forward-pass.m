#include <mulle-objc-runtime/mulle-objc-runtime.h>
// method in a "Forward" category: no warning
@interface Foo
@end

@interface Foo (Forward)
- (int) value;
@end

@interface Foo (Bar)
- (int) value;
@end

@implementation Foo
@end

@implementation Foo (Bar)
- (int) value { return 42; }
@end

int main(void) { return 0; }
