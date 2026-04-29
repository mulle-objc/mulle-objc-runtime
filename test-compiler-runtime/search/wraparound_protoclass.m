#ifndef __MULLE_OBJC__
# import <Foundation/Foundation.h>
#else
# import <mulle-objc-runtime/mulle-objc-runtime.h>
#endif


@protocol_class A;
@protocol_interface A
@end

@protocol_implementation A
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
+ (void) z
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
@end


@interface B (Forward)
+ (void) x;
+ (void) y;
@end


int   main()
{
   [B x];
   [B y];
   [B z];
   return( 0);
}
