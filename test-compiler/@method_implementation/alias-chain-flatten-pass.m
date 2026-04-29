#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

// retain is defined in another TU.  The compiler must flatten the chain
// so that both -copy and -immutableCopy directly alias -retain (not each
// other), avoiding any load-order dependency in the runtime.
@interface Foo
- (instancetype) retain;
- (id) copy;
- (id) immutableCopy;
@end

@implementation Foo
@method_implementation -copy          = -retain;
@method_implementation -immutableCopy = -copy;
@end

int main(void) { return 0; }
