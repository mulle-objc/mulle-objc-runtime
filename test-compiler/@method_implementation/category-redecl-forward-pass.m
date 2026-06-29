#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"
#pragma clang diagnostic error "-Wmulle-method-implementation"

//
// Test that a category adopting MulleObjCForwarding does not produce a
// "method is already declared in category" warning when another
// category implements the method.
//

@protocol MulleObjCForwarding
@end

@interface Foo
@end

@interface Foo( Forwarding) <MulleObjCForwarding>
- (int) value;
@end

@interface Foo( Bar)
- (int) value;
@end

@implementation Foo
@end

@implementation Foo( Bar)
- (int) value { return 42; }
@end

int main(void) { return 0; }
