#include <mulle-objc-runtime/mulle-objc-runtime.h>



@class A;
@protocol A
@end

@class B;
@protocol B
@end


@interface Foo < A, B>
@end

@implementation Foo

+ (void) initialize
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}


+ (void) deinitialize
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}

+ (void) nop
{

}

@end

@interface A <A>
@end
@implementation A
+ (void) initialize
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}

+ (void) deinitialize
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
@end

@interface B <B>
@end
@implementation B
+ (void) initialize
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}

+ (void) deinitialize
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
@end



int   main( void)
{
   [Foo nop];
   return( 0);
}
