#include <mulle-objc-runtime/mulle-objc-runtime.h>



@protocol_class A;
@protocol_interface A
@end

@protocol_class B;
@protocol_interface B
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

@protocol_implementation A
+ (void) initialize
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}

+ (void) deinitialize
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
@end

@protocol_implementation B
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
