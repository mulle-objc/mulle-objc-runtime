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


@class B;
@protocol B
@end
@interface B <B>
@end


@interface C < A, B>
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


@implementation B
+ (void) x
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) y
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
@end


@implementation C
+ (void) z
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
@end


int   main()
{
   [C x];
   [C y];
   [C z];
}