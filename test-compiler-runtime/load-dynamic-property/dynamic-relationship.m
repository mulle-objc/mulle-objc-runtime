#include <mulle-objc-runtime/mulle-objc-runtime.h>


@interface Foo

@property( assign, relationship, dynamic) int  value;

@end


@implementation Foo

@dynamic value;

+ (Class) class
{
   return( self);
}


@end


int   main( void)
{
   Class                          cls;
   struct _mulle_objc_property    *property;
   struct _mulle_objc_universe    *universe;
   struct _mulle_objc_ivar        *ivar;
   struct _mulle_objc_descriptor  *desc;
   mulle_objc_methodid_t          sel;

   cls      = [Foo class];
   universe = _mulle_objc_infraclass_get_universe( cls);

   property = mulle_objc_infraclass_search_property( cls, @selector( value));
   printf( "property was %sfound\n", property ? "" : "not ");

   if( property)
   {
      printf( "\"%s\" (%s) is %sdynamic\n",
                     _mulle_objc_property_get_name( property),
                     _mulle_objc_property_get_signature( property),
                     _mulle_objc_property_is_dynamic( property) ? "" : "not ");

      sel  = _mulle_objc_property_get_getter( property);
      printf( "getter %08x\n", sel);
      desc = _mulle_objc_universe_lookup_descriptor( universe, sel);
      printf( "method \"-%s\" (%s)\n",
                  desc ? _mulle_objc_descriptor_get_name( desc) : "NULL",
                  desc ? _mulle_objc_descriptor_get_signature( desc) : "NULL");

      sel = _mulle_objc_property_get_setter( property);
      printf( "setter %08x\n", sel);
      desc = _mulle_objc_universe_lookup_descriptor( universe, sel);
      printf( "method \"-%s\" (%s)\n",
                  desc ? _mulle_objc_descriptor_get_name( desc) : "NULL",
                  desc ? _mulle_objc_descriptor_get_signature( desc) : "NULL");

      sel  = _mulle_objc_property_get_adder( property);
      printf( "adder %08x\n", sel);
      desc = _mulle_objc_universe_lookup_descriptor( universe, sel);
      printf( "method \"-%s\" (%s)\n",
                  desc ? _mulle_objc_descriptor_get_name( desc) : "NULL",
                  desc ? _mulle_objc_descriptor_get_signature( desc) : "NULL");

      sel  = _mulle_objc_property_get_remover( property);
      printf( "remover %08x\n", sel);
      desc = _mulle_objc_universe_lookup_descriptor( universe, sel);
      printf( "method \"-%s\" (%s)\n",
                  desc ? _mulle_objc_descriptor_get_name( desc) : "NULL",
                  desc ? _mulle_objc_descriptor_get_signature( desc) : "NULL");
   }

   return( 0);
}
