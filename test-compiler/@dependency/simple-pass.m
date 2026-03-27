#include <mulle-objc-runtime/mulle-objc-runtime.h>

// simplest case: single impl-scope @dependency
@interface Foo
@end

@implementation Foo
@dependency Local;
@end

int main(void) { return 0; }
