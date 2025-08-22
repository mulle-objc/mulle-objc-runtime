#include <mulle-objc-runtime/mulle-objc-runtime.h>


@interface Bar

+ (void) a:(id) a;
+ (void) a:(id) a
         b:(id) b;
@end


@implementation Bar

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


@interface Foo : Bar
@end


@implementation Foo

+ (void)  a:(id) a
{
   [super a:a];
}


+ (void) a:(id) a
         b:(id) b
{
   [super a:a
          b:b];
}

@end


int   main( void)
{
   [Foo a:(id) 0];
   [Foo a:(id) 0 b:(id) 1];
   return( 0);
}
