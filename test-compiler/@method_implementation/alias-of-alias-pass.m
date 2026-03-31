#include <mulle-objc-runtime/mulle-objc-runtime.h>

// alias of an alias — should work if both are in the same impl
@interface Foo
- (void)a;
- (void)b;
- (void)c;
@end

@implementation Foo
- (void)a {}
@method_implementation -b = -a;
@method_implementation -c = -a;
@end

int main(void) { return 0; }
