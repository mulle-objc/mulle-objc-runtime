#include <mulle-objc-runtime/mulle-objc-runtime.h>



@mixin A;
@mixin A
@end

@mixin B;
@mixin B
@end


@interface Foo < A, B>
@end

@implementation Foo

+ (void) initialize
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}


+ (void) deinitialize
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}

+ (void) nop
{

}

@end

@implementation A
+ (void) initialize
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}

+ (void) deinitialize
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
@end

@implementation B
+ (void) initialize
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}

+ (void) deinitialize
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
@end



int   main( void)
{
   [Foo nop];
   return( 0);
}
