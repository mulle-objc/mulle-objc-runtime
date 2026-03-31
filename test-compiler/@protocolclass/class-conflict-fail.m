#include <mulle-objc-runtime/mulle-objc-runtime.h>

// @class before @protocolclass is an error
@class Foo;
@protocolclass Foo;
