#include <mulle-objc-runtime/mulle-objc-runtime.h>

@protocol_interface Foo
- (void) doSomething;
@end

@protocol_implementation Foo
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
   [super doSomething];
}

@end

int   main( void)
{
   return 0;
}
