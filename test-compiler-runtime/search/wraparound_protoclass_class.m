#ifndef __MULLE_OBJC__
# import <Foundation/Foundation.h>
#else
# import <mulle-objc-runtime/mulle-objc-runtime.h>
#endif


@mixin A;
@mixin A
@end

@implementation A
+ (void) x
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) y
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
@end



@interface B < A>
@end


@implementation B
- (void) w
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
+ (void) z
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
@end


@interface C : B
@end


@implementation C
@end


@interface C (Forward)
+ (void) x;
+ (void) y;
@end


int   main()
{
   [C w];
   [C x];
   [C y];
   [C z];
   return( 0);
}
