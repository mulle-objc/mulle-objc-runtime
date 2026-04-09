#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

// class method alias
@interface Foo
+ (void)initialize;
+ (void)load;
@end

@implementation Foo
+ (void)initialize {}
@method_implementation +load = +initialize;
@end

int main(void) { return 0; }
