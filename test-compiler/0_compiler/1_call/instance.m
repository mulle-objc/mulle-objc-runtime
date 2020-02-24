#include <mulle-objc-runtime/mulle-objc-runtime.h>


@interface Foo

+ (instancetype) alloc;
- (void) dealloc;

- (void) a:(id) a;
- (void) a:(id) a
         b:(id) b;
@end


@implementation Foo

+ (instancetype) alloc
{
   return( mulle_objc_infraclass_alloc_instance( self));
}

- (void) dealloc
{
   mulle_objc_instance_free( self);
}

- (void)  a:(id) a
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}


- (void) a:(id) a
         b:(id) b
{
   printf( "%s\n", __PRETTY_FUNCTION__);
}


@end


int   main( void)
{
   Foo  *foo;

   foo = [Foo alloc];

   [Foo a:(id) 0];
   [Foo a:(id) 0 b:(id) 1];

   [foo dealloc];

   return( 0);
}
