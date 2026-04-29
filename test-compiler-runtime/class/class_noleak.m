#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"

@protocol_class Bar;
@protocol_interface Bar
@end

@interface Foo < Bar>
@end

@interface Foobar : Foo
@end

@interface Foobar( Whatevs)
@end


@implementation Foobar
@end

@implementation Foobar( Whatevs)
@end

@implementation Foo
@end

@protocol_implementation Bar
@end


int   main( void)
{
   return( 0);
}
