#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

// @dependency suppresses the warning
@interface Foo
@end

@interface Foo (Base)
- (int) value;
@end

@interface Foo (Bar)
- (int) value;
@end

@implementation Foo
@end

@implementation Foo (Bar)
@dependency Foo(Base);
- (int) value { return 42; }
@end

int main(void) { return 0; }
