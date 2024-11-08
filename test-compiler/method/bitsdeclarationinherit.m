#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>


//
// this test shows: that objc_user_2 is inherited by both whatever
// methods, but additional attributes work too
//
@interface Foo

- (void) whatever  __attribute__((annotate("objc_user_2")));

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



@interface Bar : Foo
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
   return( 1); // force error
}

