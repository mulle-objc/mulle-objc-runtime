#include <mulle-objc-runtime/mulle-objc-runtime.h>

// @protocol definition after @mixin is an error
@mixin Qux;
@protocol Qux < Qux>
@end
