#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>


@interface Bar
{
   uint32_t  spacer[ 4];
}

// mulle-clang 9 feature
@property( observable, retain) id   a;

@end


@implementation Bar

+ (Class) class
{
   return( self);
}

- (void) willChange
{
   printf( "%s\n", __FUNCTION__);
}


@end


int   main( void)
{
   struct _mulle_objc_infraclass   *BarClass;
   struct _mulle_objc_property     *property;
   id                              obj;

   BarClass  = (struct _mulle_objc_infraclass *) [Bar class];
   property  = mulle_objc_infraclass_search_property( BarClass,
                                                     @selector( a));
   if( ! property)
   {
      fprintf( stderr, "property \"a\" not found\n");
      return( -1);
   }

   printf( "%x\n", _mulle_objc_property_get_bits( property));
   printf( "%s\n", _mulle_objc_property_get_signature( property));

   obj = mulle_objc_infraclass_alloc_instance( BarClass);

   [obj setA:(id) 0];

   mulle_objc_instance_free( obj);

   return( 0);
}
