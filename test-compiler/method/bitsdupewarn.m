#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>


@interface Foo
@end


@implementation Foo

+ (void) load
{
   printf( "%s\n", __FUNCTION__);
}

- (void) whatever  __attribute__((annotate("objc_user_0")))
{
}

@end



@interface Bar
@end


@implementation Bar

+ (void) load
{
   printf( "%s\n", __FUNCTION__);
}

- (void) whatever  __attribute__((annotate("objc_user_1")))
{
}

@end



int  main( void)
{
   return( 0);
}

