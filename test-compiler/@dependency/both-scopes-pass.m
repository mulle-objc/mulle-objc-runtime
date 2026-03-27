#include <mulle-objc-runtime/mulle-objc-runtime.h>

// both file-scope and impl-scope @dependency in the same TU
@dependency Shared;

@interface Foo
@end

@implementation Foo
@dependency Local;
@end

int main(void) { return 0; }
