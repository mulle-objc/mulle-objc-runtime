// Foo ==  40413ff3
#define MULLE_OBJC_FASTCLASSHASH_31   0x40413ff3

#include <mulle-objc-runtime/mulle-objc-runtime.h>

#define MULLE_OBJC_FASTCLASSID_31  MULLE_OBJC_CLASSID( MULLE_OBJC_FASTCLASSHASH_31)

@interface Foo

+ (id) new;
- (id) init;

@end


@implementation Foo

+ (id) new
{
   return( [mulle_objc_infraclass_alloc_instance( self) init]);
}


- (id) init
{
   printf( "Foo\n");
   return( self);
}

@end


main()
{
   Foo  *foo;
   struct _mulle_objc_universe    *universe;

   universe = mulle_objc_global_get_universe( __MULLE_OBJC_UNIVERSEID__);

   printf( "%d\n", mulle_objc_get_fastclasstable_index( MULLE_OBJC_FASTCLASSID_31));

   mulle_objc_universe_dotdump_to_directory( universe, ".");

   foo = [Foo new];
}
