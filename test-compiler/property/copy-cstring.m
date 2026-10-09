#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <string.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"


@interface Foo
{
   char          *_name;
   const char    *_cname;
   unsigned char *_uname;
}
@property( copy) char          *name;
@property( copy) const char    *cname;
@property( copy) unsigned char *uname;
@end


@implementation Foo

+ (id) new
{
   return( mulle_objc_infraclass_alloc_instance( (struct _mulle_objc_infraclass *) self));
}


+ (Class) class
{
   return( self);
}


@synthesize name  = _name;
@synthesize cname = _cname;
@synthesize uname = _uname;

@end


static void   print_property( Class cls,
                              mulle_objc_propertyid_t propertyid)
{
   struct _mulle_objc_property   *property;

   property = mulle_objc_infraclass_search_property( (struct _mulle_objc_infraclass *) cls,
                                                     propertyid);
   if( ! property)
      exit( 1);

   printf( "%s: %x\n",
           _mulle_objc_property_get_name( property),
           _mulle_objc_property_get_bits( property));
}


int   main( void)
{
   Foo    *foo;
   char   buf[ 64];

   print_property( [Foo class], @selector( name));
   print_property( [Foo class], @selector( cname));
   print_property( [Foo class], @selector( uname));

   foo = [Foo new];

   strcpy( buf, "hello");
   [foo setName:buf];
   strcpy( buf, "xxxxx");
   printf( "name: %s, %s\n",
           [foo name],
           [foo name] == buf ? "aliased" : "duplicated");

   strcpy( buf, "c-hello");
   [foo setCname:buf];
   strcpy( buf, "xxxxxxx");
   printf( "cname: %s, %s\n",
           [foo cname],
           [foo cname] == buf ? "aliased" : "duplicated");

   strcpy( buf, "u-hello");
   [foo setUname:(unsigned char *) buf];
   strcpy( buf, "xxxxxxx");
   printf( "uname: %s, %s\n",
           (char *) [foo uname],
           (char *) [foo uname] == buf ? "aliased" : "duplicated");

   [foo setName:NULL];
   printf( "name is %s\n", [foo name] ? [foo name] : "NULL");
   [foo setCname:NULL];
   printf( "cname is %s\n", [foo cname] ? [foo cname] : "NULL");
   [foo setUname:NULL];
   printf( "uname is %s\n", [foo uname] ? (char *) [foo uname] : "NULL");

   return( 0);
}
