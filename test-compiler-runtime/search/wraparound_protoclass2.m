#ifndef __MULLE_OBJC__
# import <Foundation/Foundation.h>
#else
# import <mulle-objc-runtime/mulle-objc-runtime.h>
#endif


// protocolclass A
@protocol_class A;
@protocol_interface A
@end

// protocolclass B
@protocol_class B;
@protocol_interface B
@end

// class C inherits two protocolclasses
@interface C < A, B>
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


@protocol_implementation B
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
+ (void) z
{
   mulle_printf( "%s\n", __PRETTY_FUNCTION__);
}
@end


@interface C (Forward)
+ (void) x;
+ (void) y;
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
   // 'A' has no superclass, so stop and wrap at last from metaclass 'C' to infraclass 'C'
   // 'B' has -y  -> FINE
   [C y];

   // +z is a class method
   // 'C' has +x -> FINE
   [C z];
   return( 0);
}
