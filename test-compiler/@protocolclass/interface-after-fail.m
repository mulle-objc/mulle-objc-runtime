#include <mulle-objc-runtime/mulle-objc-runtime.h>

// @interface after @protocol_class is an error
@protocol_class Baz;
@interface Baz
@end
