#include <mulle-objc-runtime/mulle-objc-runtime.h>
// return type mismatch: -(void)foo vs -(int)bar
@interface Foo
- (void)foo:(int)x;
- (int)bar:(char *)s;
@end

@implementation Foo
- (int)bar:(char *)s { return 0; }
@method_implementation -foo: = -bar:;
@end
