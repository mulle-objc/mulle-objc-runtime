#ifndef __MULLE_OBJC__
# import <Foundation/Foundation.h>
#else
# import <mulle-objc-runtime/mulle-objc-runtime.h>
#endif


@protocol_class A;
@protocol_interface A
@end

@protocol_implementation A
+ (void) a
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) a
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
+ (void) ab
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) ab
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
+ (void) ac
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) ac
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
+ (void) ad
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) ad
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
@end


@protocol_class B;
@protocol_interface B
@end

@protocol_implementation B
+ (void) b
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) b
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
+ (void) ab
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) ab
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
+ (void) bc
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) bc
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
+ (void) bd
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) bd
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
@end


@interface C < A, B>
@end


@implementation C
+ (void) c
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) c
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
+ (void) ac
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) ac
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
+ (void) bc
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) bc
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
+ (void) cd
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) cd
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
@end


@interface D : C
@end


@implementation D
+ (void) d
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) d
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
+ (void) ad
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) ad
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
+ (void) bd
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) bd
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
+ (void) cd
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) cd
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
@end


@interface D (Forward)
+ (void) a;
+ (void) ab;
+ (void) b;
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
