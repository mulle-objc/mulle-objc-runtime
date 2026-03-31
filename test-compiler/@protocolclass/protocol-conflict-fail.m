#include <mulle-objc-runtime/mulle-objc-runtime.h>

// @protocol before @protocolclass is an error
@protocol Bar;
@protocolclass Bar;
