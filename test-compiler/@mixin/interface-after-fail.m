#include <mulle-objc-runtime/mulle-objc-runtime.h>

// @interface after @mixin is an error
@mixin Baz;
@interface Baz
@end
