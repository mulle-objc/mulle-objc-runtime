#include <mulle-objc-runtime/mulle-objc-runtime.h>

// @protocol definition after @protocolclass is an error
@protocolclass Qux;
@protocol Qux < Qux>
@end
