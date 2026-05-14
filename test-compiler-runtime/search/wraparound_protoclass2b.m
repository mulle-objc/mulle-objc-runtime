#ifndef __MULLE_OBJC__
# import <Foundation/Foundation.h>
#else
# import <mulle-objc-runtime/mulle-objc-runtime.h>
#endif


// mixin A
@mixin A;
@mixin A
@end

// mixin B
@mixin B;
@mixin B
@end

// class C inherits two mixines
@interface C < A, B>
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


@implementation B
+ (void) x
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
- (void) y
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
@end


@implementation C
- (void) y
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
+ (void) z
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
@end


@interface C (Forward)
+ (void) x;
@end


int   main()
{
   // we call +x defined on A and B (+x is a class method)
   // 'C' don't have +x go to 'B'
   // 'B' has +x -> FINE
   [C x];

   // we call +y defined nowhere (-y is an instance method)
   // 'C' don't have +y go to 'B'
   // 'B' don't have +y go to 'A'
   // 'A' has no superclass, so wrap at last from metaclass 'A' to infraclass 'A'
   // 'A' has -y  -> FINE
   [C y];

   // +z is a class method
   // 'C' has +x -> FINE
   [C z];
   return( 0);
}
