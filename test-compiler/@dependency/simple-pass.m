#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

// simplest case: single impl-scope @dependency
@interface Foo
@end

@implementation Foo
@dependency Local;
@end

int main(void) { return 0; }
