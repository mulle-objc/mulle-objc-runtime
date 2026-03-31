#include <mulle-objc-runtime/mulle-objc-runtime.h>
// alias with return value and arguments

@interface Foo
- (NSUInteger)count;
- (NSUInteger)length;
- (void *)objectAtIndex:(NSUInteger)i;
- (void *)itemAtIndex:(NSUInteger)i;
@end

@implementation Foo
- (NSUInteger)count { return 42; }
- (void *)objectAtIndex:(NSUInteger)i { return 0; }
@method_implementation -length = -count;
@method_implementation -itemAtIndex: = -objectAtIndex:;
@end

int main(void) { return 0; }
