#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"
#pragma clang diagnostic ignored "-Wobjc-property-implementation"

//
// Test: @property(forward) emits distinct runtime bit from @property(dynamic)
//
@interface Foo
@property (forward) int bar;
@property (dynamic) int baz;
@end

@implementation Foo
@end

int   main( void)
{
   struct _mulle_objc_infraclass  *cls;
   struct _mulle_objc_property    *bar;
   struct _mulle_objc_property    *baz;
   unsigned int                    bar_bits;
   unsigned int                    baz_bits;

   cls = mulle_objc_global_lookup_infraclass_nofail( MULLE_OBJC_DEFAULTUNIVERSEID,
                                                     mulle_objc_classid_from_string( "Foo"));

   bar = _mulle_objc_infraclass_search_property( cls, mulle_objc_propertyid_from_string( "bar"));
   baz = _mulle_objc_infraclass_search_property( cls, mulle_objc_propertyid_from_string( "baz"));

   if( ! bar || ! baz)
   {
      printf( "FAIL: properties not found\n");
      return( 1);
   }

   bar_bits = _mulle_objc_property_get_bits( bar);
   baz_bits = _mulle_objc_property_get_bits( baz);

   if( ! (bar_bits & 0x100000))
   {
      printf( "FAIL: bar missing forward bit (bits=0x%x)\n", bar_bits);
      return( 1);
   }
   if( bar_bits & 0x00400)
   {
      printf( "FAIL: bar should not have dynamic bit (bits=0x%x)\n", bar_bits);
      return( 1);
   }
   if( ! (baz_bits & 0x00400))
   {
      printf( "FAIL: baz missing dynamic bit (bits=0x%x)\n", baz_bits);
      return( 1);
   }
   if( baz_bits & 0x100000)
   {
      printf( "FAIL: baz should not have forward bit (bits=0x%x)\n", baz_bits);
      return( 1);
   }

   printf( "passed\n");
   return( 0);
}
