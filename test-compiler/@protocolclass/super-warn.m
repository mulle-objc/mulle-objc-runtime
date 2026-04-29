#include <mulle-objc-runtime/mulle-objc-runtime.h>

@protocol_interface Foo
@required
- (void) doSomething;
@end

@protocol_implementation Foo
@end


@interface Root
@end

@implementation Root
@end


@interface MyClass : Root < Foo>
@end


@implementation MyClass

- (void) doSomethingElse
{
   [super doSomething];
}

@end

int   main( void)
{
   return 0;
}
