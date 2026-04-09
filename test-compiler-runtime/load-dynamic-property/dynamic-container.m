#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"


@interface Foo

@property( assign, container, dynamic) int  value;

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
   struct _mulle_objc_infraclass  *cls;
   struct _mulle_objc_property    *property;
   struct _mulle_objc_universe    *universe;
   struct _mulle_objc_ivar        *ivar;
   struct _mulle_objc_descriptor  *desc;
   mulle_objc_methodid_t          sel;

   cls      = (struct _mulle_objc_infraclass *) [Foo class];
   universe = _mulle_objc_infraclass_get_universe( cls);

   property = mulle_objc_infraclass_search_property( cls, @selector( value));
   mulle_printf( "property was %sfound\n", property ? "" : "not ");

   if( property)
   {
      mulle_printf( "\"%s\" (%s) is %sdynamic\n",
                     _mulle_objc_property_get_name( property),
                     _mulle_objc_property_get_signature( property),
                     _mulle_objc_property_is_dynamic( property) ? "" : "not ");

      sel  = _mulle_objc_property_get_getter( property);
      mulle_printf( "getter %08x\n", sel);
      desc = _mulle_objc_universe_lookup_descriptor( universe, sel);
      mulle_printf( "method \"-%s\" (%s)\n",
                  desc ? _mulle_objc_descriptor_get_name( desc) : "NULL",
                  desc ? _mulle_objc_descriptor_get_signature( desc) : "NULL");

      sel = _mulle_objc_property_get_setter( property);
      mulle_printf( "setter %08x\n", sel);
      desc = _mulle_objc_universe_lookup_descriptor( universe, sel);
      mulle_printf( "method \"-%s\" (%s)\n",
                  desc ? _mulle_objc_descriptor_get_name( desc) : "NULL",
                  desc ? _mulle_objc_descriptor_get_signature( desc) : "NULL");

      sel  = _mulle_objc_property_get_adder( property);
      mulle_printf( "adder %08x\n", sel);
      desc = _mulle_objc_universe_lookup_descriptor( universe, sel);
      mulle_printf( "method \"-%s\" (%s)\n",
                  desc ? _mulle_objc_descriptor_get_name( desc) : "NULL",
                  desc ? _mulle_objc_descriptor_get_signature( desc) : "NULL");

      sel  = _mulle_objc_property_get_remover( property);
      mulle_printf( "remover %08x\n", sel);
      desc = _mulle_objc_universe_lookup_descriptor( universe, sel);
      mulle_printf( "method \"-%s\" (%s)\n",
                  desc ? _mulle_objc_descriptor_get_name( desc) : "NULL",
                  desc ? _mulle_objc_descriptor_get_signature( desc) : "NULL");
   }

   return( 0);
}
