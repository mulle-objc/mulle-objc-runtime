// Hackery for low level test don't do this otherwise
#ifdef __MULLE_OBJC_TPS__
# undef __MULLE_OBJC_TPS__
#endif
#define __MULLE_OBJC_NO_TPS__   1


#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <mulle-objc-runtime/mulle-objc-lldb.h>


@interface SomeString
{
   char       *_s;
   intptr_t   _len;
}
@end


@implementation SomeString

+ (Class) class
{
   return( self);
}


- (void) print
{
   printf( "%d: %s\n", (int) self->_len, self->_s);
}

- (void) dealloc
{
   struct mulle_allocator           *allocator;
   struct _mulle_objc_infraclass    *infra;
   struct _mulle_objc_objectheader  *header;

   infra     = _mulle_objc_object_get_infraclass( self);
   allocator = _mulle_objc_infraclass_get_allocator( infra);
   // this happens in a test
   if( ! allocator || ! allocator->calloc)
      allocator = &mulle_default_allocator;
   mulle_allocator_free( allocator, _mulle_objc_object_get_objectheader( self));
}

@end


main()
{
   struct _mulle_objc_universe   *universe;
   SomeString   *s;

   universe = mulle_objc_global_get_universe( 0);

   _mulle_objc_universe_set_staticstringclass( universe, [SomeString class]);

   s = mulle_objc_lldb_create_staticstring( NULL, "VfL Bochum 1848", 15, 0x08000100, 0);

   [s print];
   [s dealloc];

   return( 0);
}
