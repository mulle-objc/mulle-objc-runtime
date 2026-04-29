#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

// RHS declared in another @interface but root class (or protocolclass) 
// wants to access it, no way we error!
@interface Bar
- (id) externalMethod;
@end

@interface Foo
- (id) copy;
@end

@implementation Foo
@method_implementation -copy = -externalMethod;
@end

int main(void) { return 0; }
