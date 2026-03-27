#include <mulle-objc-runtime/mulle-objc-runtime.h>

// multiple @dependency directives in a single @implementation
@interface Foo
@end

@implementation Foo
@dependency Local;
@dependency Foundation(Base);
@dependency Bar;
@end

int main(void) { return 0; }
