#include <mulle-objc-runtime/mulle-objc-runtime.h>


@interface Foo

+ (void) a:(id) a;
+ (void) a:(id) a
         b:(id) b;
@end


@implementation Foo

+ (void)  a:(id) a
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}


+ (void) a:(id) a
         b:(id) b
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}


@end


int   main( void)
{
   [Foo a:(id) 0];
   [Foo a:(id) 0 b:(id) 1];
   return( 0);
}
