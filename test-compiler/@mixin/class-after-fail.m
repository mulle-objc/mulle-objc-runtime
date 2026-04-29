#include <mulle-objc-runtime/mulle-objc-runtime.h>

// @class after @mixin is an error
@mixin Baz;
@class Baz;
