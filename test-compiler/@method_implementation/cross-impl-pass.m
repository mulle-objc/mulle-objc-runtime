#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

// RHS declared in another @interface: silent runtime alias, no warning
@interface Bar
- (id) externalMethod;
@end

@interface Foo : Bar
- (id) copy;
@end

@implementation Foo
@method_implementation -copy = -externalMethod;
@end

int main(void) { return 0; }
