#include <mulle-objc-runtime/mulle-objc-runtime.h>

@mixin Foo
- (void) doSomething;
@end

@implementation Foo
- (void) doSomething {}
@end


@interface Root
@end

@implementation Root
@end


@interface MyClass : Root < Foo>
@end


@implementation MyClass

- (void) doSomething
{
   [super nonExistent];
}

@end

int   main( void)
{
   return 0;
}
