#define MULLE_OBJC_FASTMETHODHASH_15   0xe31996f9
#include <mulle-objc-runtime/mulle-objc-runtime.h>


#define MULLE_OBJC_FASTMETHODID_15     MULLE_OBJC_METHODID( MULLE_OBJC_FASTMETHODHASH_15)


@interface Foo

+ (id) new;
- (void) whatever;

@end


@implementation Foo

+ (id) new
{
   return( mulle_objc_infraclass_alloc_instance( self));
}


- (void) whatever
{
   printf( "whatever\n");
}

@end


main()
{
   Foo  *foo;

   printf( "%d\n", mulle_objc_get_fastmethodtable_index( MULLE_OBJC_FASTMETHODID_15));
   foo = [Foo new];
   mulle_objc_object_call( foo, MULLE_OBJC_FASTMETHODID_15, foo);
}


