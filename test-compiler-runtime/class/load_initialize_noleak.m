#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"


@interface Foo
@end

@interface Foobar : Foo
@end

@interface Foobar( Whatevs)
@end



@implementation Foobar

+ (void) load
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}

+ (void) nop
{
}

@end


@implementation Foobar( Whatevs)

+ (void) initialize
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}

@end


@implementation Foo

+ (void) load
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}

+ (void) initialize
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}

@end



int   main( void)
{
   [Foobar nop];
   return( 0);
}
