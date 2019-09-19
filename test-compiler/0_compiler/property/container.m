#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>


@interface Container
{
   id             _contents[ 16];
   unsigned int   _n;
};
@end


@implementation Container

+ (Class) class
{
   return( self);
}

- (void) addObject:(id) a
{
   printf( "%s\n", __FUNCTION__);
}


- (void) removeObject:(id) a
{
   printf( "%s\n", __FUNCTION__);
}

@end



@interface Bar
{
   uint32_t  spacer[ 4];
}

// mulle-clang 9 feature
@property( container, retain) id   a;

@end


@implementation Bar

+ (Class) class
{
   return( self);
}

@end


int   main( void)
{
   struct _mulle_objc_infraclass   *BarClass;
   struct _mulle_objc_infraclass   *ContainerClass;
   struct _mulle_objc_property     *property;
   id                              obj;
   id                              container;

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

   ContainerClass  = (struct _mulle_objc_infraclass *) [Container class];
   container       = mulle_objc_infraclass_alloc_instance( ContainerClass);
   obj             = mulle_objc_infraclass_alloc_instance( BarClass);

   [obj setA:container];
   [obj addToA:(id) 18];
   [obj removeFromA:(id) 19];

   mulle_objc_object_free( obj);
   mulle_objc_object_free( container);

   return( 0);
}
