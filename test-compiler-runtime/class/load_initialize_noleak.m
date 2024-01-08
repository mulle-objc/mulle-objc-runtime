#include <mulle-objc-runtime/mulle-objc-runtime.h>


@interface Foo
@end

@interface Foobar : Foo
@end

@interface Foobar( Whatevs)
@end



@implementation Foobar

+ (void) load
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}

+ (void) nop
{
}

@end


@implementation Foobar( Whatevs)

+ (void) initialize
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}

@end


@implementation Foo

+ (void) load
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}

+ (void) initialize
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}

@end



int   main( void)
{
   [Foobar nop];
   return( 0);
}
