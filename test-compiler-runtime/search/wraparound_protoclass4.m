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
+ (void) a
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) ab
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) ac
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) ad
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
@end


@class B;
@protocol B
@end
@interface B <B>
@end


@implementation B
+ (void) b
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) ab
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) bc
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) bd
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
@end


@interface C < A, B>
@end


@implementation C
+ (void) c
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) ac
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) bc
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) cd
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
@end


@interface D : C
@end


@implementation D
+ (void) d
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) ad
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) bd
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) cd
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}
@end


int   main()
{
   [D a];
   [D ab];
   [D ac];
   [D ad];

   [D b];
//   [D ab];
   [D bc];
   [D bd];

   [D c];
//   [D ac];
//   [D bc];
   [D cd];

   [D d];
//   [D ad];
//   [D bd];
//   [D cd];
   return( 0);
}
