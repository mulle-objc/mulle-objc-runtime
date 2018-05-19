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
   return( [mulle_objc_infraclass_alloc_instance( self, NULL) init]);
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

   printf( "%d\n", mulle_objc_get_fastclasstable_index( MULLE_OBJC_FASTCLASSID_31));

   mulle_objc_dotdump_universe_to_tmp();

   foo = [Foo new];
}
