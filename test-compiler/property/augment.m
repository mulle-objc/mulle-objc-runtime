#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>


@interface Foo

@property( assign) int  value;

@end


@interface Bar : Foo

@property( assign, observable) int  value;

@end


@implementation Foo

+ (struct _mulle_objc_infraclass *) class
{
   return( (struct _mulle_objc_infraclass *) self);
}

@end


@implementation Bar

@synthesize value;      // need to resyntesize accessors

- (void) willChange
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}

@end


static void  test( struct _mulle_objc_infraclass *cls)
{
   Foo     *obj;

   obj = mulle_objc_infraclass_alloc_instance( cls);
   printf( "%s:\n", _mulle_objc_infraclass_get_name( cls));
   [obj setValue:18];
   printf( "   value=%d\n", [obj value]);
   mulle_objc_instance_free( obj);
}


int   main( int argc, char *argv[])
{
   test( [Foo class]);
   test( [Bar class]);

   return( 0);
}
