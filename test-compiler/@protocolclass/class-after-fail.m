#include <mulle-objc-runtime/mulle-objc-runtime.h>

// @class after @protocolclass is an error
@protocolclass Baz;
@class Baz;
