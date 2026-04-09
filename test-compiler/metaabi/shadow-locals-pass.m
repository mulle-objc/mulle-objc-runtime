#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

// shadow locals: multi-arg method params accessible by name
// verifies that 'a' and 'b' resolve to shadow VarDecls (not _param->a)

@interface Foo
- (int)addX:(int)a toY:(int)b;
@end

@implementation Foo
- (int)addX:(int)a toY:(int)b {
    return a + b;
}
@end

int main(void) { return 0; }
