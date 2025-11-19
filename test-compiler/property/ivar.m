#include <mulle-objc-runtime/mulle-objc-runtime.h>



@interface Foo
{
   char  _a;
}
@property char a;
@property int b;

+ (Class) class;

@end

@implementation Foo

+ (Class) class
{
   return( self);
}

@end


int   main( void)
{
   Class   cls;
   struct _mulle_objc_ivar       *ivar;
   struct _mulle_objc_property   *property;

   cls = [Foo class];

   property = _mulle_objc_infraclass_search_property( cls, @selector( a));
   ivar     = _mulle_objc_infraclass_search_ivar( cls, @selector( _a));

   if( _mulle_objc_ivar_get_ivarid( ivar) != _mulle_objc_property_get_ivarid( property))
      return( 1);

   property = _mulle_objc_infraclass_search_property( cls, @selector( b));
   ivar     = _mulle_objc_infraclass_search_ivar( cls, @selector( _b));

   if( _mulle_objc_ivar_get_ivarid( ivar) != _mulle_objc_property_get_ivarid( property))
      return( 1);

   return( 0);
}
