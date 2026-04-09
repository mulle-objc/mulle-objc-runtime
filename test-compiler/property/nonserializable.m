#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"


@interface Bar

@property( nonserializable) id b;
@property( serializable) id c;

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

   printf( "%s:", _mulle_objc_property_get_name( property));
   printf( " %x ", _mulle_objc_property_get_bits( property));
   printf( "\"%s\"\n", _mulle_objc_property_get_signature( property));
}

int   main( void)
{
   Class   cls;

   cls = [Bar class];
   print_property( cls, @selector( b));
   print_property( cls, @selector( c));
   return( 0);
}
