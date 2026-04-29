#include <mulle-objc-runtime/mulle-objc-runtime.h>

// @protocol definition after @protocol_class is an error
@protocol_class Qux;
@protocol Qux < Qux>
@end
