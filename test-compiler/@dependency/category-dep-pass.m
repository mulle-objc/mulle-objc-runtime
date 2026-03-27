#include <mulle-objc-runtime/mulle-objc-runtime.h>

// @dependency with a category name: @dependency ClassName(CategoryName)
@interface Foo
@end

@implementation Foo
@dependency Foundation(Base);
@end

int main(void) { return 0; }
