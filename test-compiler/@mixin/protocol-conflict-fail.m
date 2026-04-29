#include <mulle-objc-runtime/mulle-objc-runtime.h>

// @protocol before @mixin is an error
@protocol Bar;
@mixin Bar;
