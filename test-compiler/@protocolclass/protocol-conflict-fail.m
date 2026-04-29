#include <mulle-objc-runtime/mulle-objc-runtime.h>

// @protocol before @protocol_class is an error
@protocol Bar;
@protocol_class Bar;
