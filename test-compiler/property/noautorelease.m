#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>


@interface Bar

@property( noautorelease) id b;
@property( autorelease) id c;

@end


@implementation Bar

+ (Class) class
{
   return( self);
}

@end


static void   print_property( Class cls, 
                              mulle_objc_propertyid_t propertyid)
{
   struct _mulle_objc_property   *property;

   property = mulle_objc_infraclass_search_property( (struct _mulle_objc_infraclass *) cls, 
                                                     propertyid);
   if( ! property)
      exit( 1);

   mulle_printf( "%s:", _mulle_objc_property_get_name( property));
   mulle_printf( " %x ", _mulle_objc_property_get_bits( property));
   mulle_printf( "\"%s\"\n", _mulle_objc_property_get_signature( property));
}

int   main( void)
{
   Class   cls;

   cls = [Bar class];
   print_property( cls, @selector( b));
   print_property( cls, @selector( c));
   return( 0);
}
