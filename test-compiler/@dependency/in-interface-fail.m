#include <mulle-objc-runtime/mulle-objc-runtime.h>

// @dependency inside @interface must be rejected
@interface Foo
@dependency Local;
@end

@implementation Foo
@end
