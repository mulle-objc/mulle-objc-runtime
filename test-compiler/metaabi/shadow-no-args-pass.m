#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

// shadow locals: no args — no shadow needed, just verify no crash
@interface Foo
- (int)count;
@end

@implementation Foo
- (int)count {
    return 42;
}
@end

int main(void) { return 0; }
