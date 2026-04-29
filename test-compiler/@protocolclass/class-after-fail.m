#include <mulle-objc-runtime/mulle-objc-runtime.h>

// @class after @protocol_class is an error
@protocol_class Baz;
@class Baz;
