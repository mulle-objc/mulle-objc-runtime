#include <mulle-objc-runtime/mulle-objc-runtime.h>



@interface Foo

@property( assign)           int i;
@property( assign)           char  *p1;
@property( assign)           id  a1;
@property( retain)           id  b1;
@property( copy)             id  c1;
@property( assign, readonly) id  a2;
@property( retain, readonly) id  b2;
@property( copy, readonly)   id  c2;
@property( assign, nonnull)  id  a3;
@property( retain, nonnull)  id  b3;
@property( copy, nonnull)    id  c3;
@property( assign, readonly, dynamic)  id  a4;
@property( retain, readonly, dynamic)  id  b4;
@property( copy, readonly, dynamic)    id  c4;
@property( assign, dynamic)  id  a5;
@property( retain, dynamic)  id  b5;
@property( copy, dynamic)    id  c5;

@end


@implementation Foo

+ (id) new
{
   return( mulle_objc_infraclass_alloc_instance( self));
}


+ (Class) class
{
   return( self);
}

- (id) a4
{
   return( 0);
}
- (id) b4
{
   return( 0);
}
- (id)  c4
{
   return( 0);
}


- (id) a5
{
   return( 0);
}
- (id) b5
{
   return( 0);
}
- (id)  c5
{
   return( 0);
}
- (void) setA5:(id) unused
{
}
- (void) setB5:(id) unused
{
}
- (void) setC5:(id) unused
{
}


@end


static void   dump_property_with_id( Class cls, mulle_objc_propertyid_t pid)
{
   struct _mulle_objc_property   *property;

   property = _mulle_objc_infraclass_search_property( cls, pid);
   if( ! property)
      abort();

   printf( "property %s has %s getter, %s setter, 0x%x bits\n",
               property->name,
               property->getter ? "a" : "no",
               property->setter ? "a" : "no",
               property->bits);
}


int   main( void)
{
   Class  cls;

   cls = [Foo class];
   dump_property_with_id( cls, @selector( i));
   dump_property_with_id( cls, @selector( p1));

   dump_property_with_id( cls, @selector( a1));
   dump_property_with_id( cls, @selector( b1));
   dump_property_with_id( cls, @selector( c1));

   dump_property_with_id( cls, @selector( a2));
   dump_property_with_id( cls, @selector( b2));
   dump_property_with_id( cls, @selector( c2));

   dump_property_with_id( cls, @selector( a3));
   dump_property_with_id( cls, @selector( b3));
   dump_property_with_id( cls, @selector( c3));

   dump_property_with_id( cls, @selector( a4));
   dump_property_with_id( cls, @selector( b4));
   dump_property_with_id( cls, @selector( c4));

   dump_property_with_id( cls, @selector( a5));
   dump_property_with_id( cls, @selector( b5));
   dump_property_with_id( cls, @selector( c5));

   return( 0);
}
