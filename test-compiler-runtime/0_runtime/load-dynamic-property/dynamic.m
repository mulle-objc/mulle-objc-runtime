#include <mulle-objc-runtime/mulle-objc-runtime.h>


@interface Foo

@property( assign, dynamic) int  value;

- (int) example;
- (void) setExample:(int) x;

@end


@implementation Foo

@dynamic value;

+ (Class) class
{
   return( self);
}

- (int) example
{
}

- (void) setExample:(int) x
{
}

@end


main()
{
   Class                          cls;
   struct _mulle_objc_property    *property;
   struct _mulle_objc_universe    *universe;
   struct _mulle_objc_ivar        *ivar;
   struct _mulle_objc_descriptor  *desc;
   struct _mulle_objc_descriptor  *getterReference;
   struct _mulle_objc_descriptor  *setterReference;
   mulle_objc_methodid_t          sel;

   cls      = [Foo class];
   universe = _mulle_objc_infraclass_get_universe( cls);

   getterReference = _mulle_objc_universe_lookup_descriptor( universe, @selector( example));
   printf( "getter reference \"-%s\" (%s)\n",
                  getterReference ? _mulle_objc_descriptor_get_name( getterReference) : "NULL",
                  getterReference ? _mulle_objc_descriptor_get_signature( getterReference) : "NULL");

   setterReference = _mulle_objc_universe_lookup_descriptor( universe, @selector( setExample:));
   printf( "setter reference \"-%s\" (%s)\n",
                  setterReference ? _mulle_objc_descriptor_get_name( setterReference) : "NULL",
                  setterReference ? _mulle_objc_descriptor_get_signature( setterReference) : "NULL");


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

      if( property->ivarid)
      {
         ivar  = _mulle_objc_infraclass_search_ivar( cls, property->ivarid);
         printf( "ivar %s\n", ivar ? _mulle_objc_ivar_get_name( ivar) : "NULL");
      }
      else
         printf( "no ivar\n");

      sel = _mulle_objc_property_get_setter( property);
      printf( "setter %08x\n", sel);
      desc = _mulle_objc_universe_lookup_descriptor( universe, sel);
      printf( "method \"-%s\" (%s)\n",
                  desc ? _mulle_objc_descriptor_get_name( desc) : "NULL",
                  desc ? _mulle_objc_descriptor_get_signature( desc) : "NULL");
   }

   return( 0);
}
