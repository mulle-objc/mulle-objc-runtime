#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>


@interface Bar

@property( serializable) char b;
@property char c;

@end


@implementation Bar

+ (Class) class
{
   return( self);
}

@end


main()
{
   Class                         cls;
   struct _mulle_objc_property   *property;

   cls      = [Bar class];
   property = mulle_objc_infraclass_search_property( cls, @selector( b));
   if( ! property)
      return( -1);

   printf( "%x\n", _mulle_objc_property_get_bits( property));
   printf( "%s\n", _mulle_objc_property_get_signature( property));

   property = mulle_objc_infraclass_search_property( cls, @selector( c));
   if( ! property)
      return( -1);

   printf( "%x\n", _mulle_objc_property_get_bits( property));
   printf( "%s\n", _mulle_objc_property_get_signature( property));

   return( 0);
}
