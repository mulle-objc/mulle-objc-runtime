#include <mulle-objc-runtime/mulle-objc-runtime.h>

// @class before @mixin is an error
@class Foo;
@mixin Foo;
