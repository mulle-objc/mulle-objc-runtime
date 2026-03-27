#include <mulle-objc-runtime/mulle-objc-runtime.h>

// file-scope @dependency: applies to all @implementations in the TU
@dependency Shared;

@interface Foo
@end

@interface Bar
@end

@implementation Foo
@end

@implementation Bar
@end

int main(void) { return 0; }
