#include <mulle-objc-runtime/mulle-objc-runtime.h>
// arity mismatch: LHS has 0 args, RHS has 1
typedef unsigned long NSUInteger;

@interface Foo
- (void)doSomething;
- (void)doSomethingWith:(NSUInteger)x;
@end

@implementation Foo
- (void)doSomethingWith:(NSUInteger)x {}
@method_implementation -doSomething = -doSomethingWith:;
@end
