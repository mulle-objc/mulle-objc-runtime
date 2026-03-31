#include <mulle-objc-runtime/mulle-objc-runtime.h>
// param type mismatch: same arity and return type, different param type
@interface Foo
- (void)foo:(int)x;
- (void)bar:(char *)s;
@end

@implementation Foo
- (void)bar:(char *)s {}
@method_implementation -foo: = -bar:;
@end
