#include <mulle-objc-runtime/mulle-objc-runtime.h>
// instance/class mismatch warning
@interface Foo
- (void)doSomething;
+ (void)classMethod;
@end

@implementation Foo
+ (void)classMethod {}
@method_implementation -doSomething = +classMethod;
@end
