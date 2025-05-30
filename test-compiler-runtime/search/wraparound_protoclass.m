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
+ (void) z
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
@end


int   main()
{
   [B x];
   [B y];
   [B z];
   return( 0);
}