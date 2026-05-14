#include <mulle-objc-runtime/mulle-objc-runtime.h>

// @implementation of @mixin without protocol definition warns
@mixin Unknown;

@implementation Unknown
- (void)doSomething {}
@end

int main(void) { return 0; }
