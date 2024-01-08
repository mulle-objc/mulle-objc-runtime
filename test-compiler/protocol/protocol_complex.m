#include <mulle-objc-runtime/mulle-objc-runtime.h>

@protocol  A      // 0x7fc56270e7a70fa8
@end

@protocol  B      // 0x9d5ed678fe57bcca
@end

@protocol  C      // 0x0d61f8370cad1d41
@end

@protocol  D      // 0xf623e75af30e62bb
@end

@protocol  E      // 0x3a3ea00cfc35332c
@end


@interface Bar <A>
@end

@interface Bar () < B >
@end

@interface Foo : Bar <C>
@end

@interface Foo () < D >
@end


@implementation Bar
@end


@implementation Foo

+ (id) new
{
   return( mulle_objc_infraclass_alloc_instance( self));
}


- (void) dealloc
{
   _mulle_objc_instance_free( (struct _mulle_objc_object *) self);
}

@end


int   main( void)
{
   Foo                            *foo;
   struct _mulle_objc_infraclass  *cls;
   struct _mulle_objc_universe    *universe;

   universe = mulle_objc_global_get_universe( __MULLE_OBJC_UNIVERSEID__);

   foo = [Foo new];
   cls = mulle_objc_object_get_infraclass( foo);
   printf( "A: %s\n",
       _mulle_objc_infraclass_conformsto_protocolid( cls,
                                              @protocol( A)) ? "YES" : "NO");
   printf( "B: %s\n",
       _mulle_objc_infraclass_conformsto_protocolid( cls,
                                              @protocol( B)) ? "YES" : "NO");
   printf( "C: %s\n",
       _mulle_objc_infraclass_conformsto_protocolid( cls,
                                              @protocol( C)) ? "YES" : "NO");
   printf( "D: %s\n",
       _mulle_objc_infraclass_conformsto_protocolid( cls,
                                              @protocol( D)) ? "YES" : "NO");
   printf( "E: %s\n",
       _mulle_objc_infraclass_conformsto_protocolid( cls,
                                              @protocol( E)) ? "YES" : "NO");

//   mulle_objc_universe_dotdump_to_directory( universe, ".");

   [foo dealloc];
   return( 0);
}
