#include <mulle-objc-runtime/mulle-objc-runtime.h>

// @class before @protocol_class is an error
@class Foo;
@protocol_class Foo;
