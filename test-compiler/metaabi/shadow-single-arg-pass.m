#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

// shadow locals: single arg that fits in void* (MetaABIVoidPtrParam path)
// this uses the direct param path, not a struct — should still work
@interface Foo
- (void)doWith:(long)x;
@end

@implementation Foo
- (void)doWith:(long)x {
    long y = x + 1;
    (void)y;
}
@end

int main(void) { return 0; }
