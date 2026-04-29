#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

// RHS declared in another @interface: class method runtime alias, no warning
@interface Bar
+ (id) externalClassMethod;
@end

@interface Foo : Bar
+ (id) copy;
@end

@implementation Foo
@method_implementation +copy = +externalClassMethod;
@end

int main(void) { return 0; }
