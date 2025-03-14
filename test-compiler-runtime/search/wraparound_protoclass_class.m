#ifndef __MULLE_OBJC__
# import <Foundation/Foundation.h>
#else
# import <mulle-objc-runtime/mulle-objc-runtime.h>
#endif


@class A;
@protocol A
@end
@interface A <A>
@end


@implementation A
+ (void) x
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) y
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
@end



@interface B < A>
@end


@implementation B
- (void) w
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
+ (void) z
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
@end


@interface C : B
@end


@implementation C
@end



int   main()
{
   [C w];
   [C x];
   [C y];
   [C z];
   return( 0);
}