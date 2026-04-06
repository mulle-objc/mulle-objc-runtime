#define MULLE_OBJC_RUNTIME_VERSION_MAJOR  0
#define MULLE_OBJC_RUNTIME_VERSION_MINOR  28
#define MULLE_OBJC_RUNTIME_VERSION_PATCH  0
#define MULLE_OBJC_RUNTIME_LOAD_VERSION   19

#include <mulle-core/mulle-core.h>

@interface NoWarn
- (void) call:(char *) s;
@end

//
// test that the compiler emits the proper code, with various optimization
// levels and flags
//
void   *mulle_objc_object_call( void *self, uint32_t sel, void *param)
{
   mulle_printf( "%s %p: %s\n", __FUNCTION__, self, (char *) param);
   return( self);
}


void   *mulle_objc_object_call_inline_minimal( void *self, uint32_t sel, void *param)
{
   mulle_printf( "%s %p: %s\n", __FUNCTION__, self, (char *) param);
   return( self);
}


void   *mulle_objc_object_call_inline_partial( void *self, uint32_t sel, void *param)
{
   mulle_printf( "%s %p: %s\n", __FUNCTION__, self, (char *) param);
   return( self);
}


void   *mulle_objc_object_call_inline( void *self, uint32_t sel, void *param)
{
   mulle_printf( "%s %p: %s\n", __FUNCTION__, self, (char *) param);
   return( self);
}


void   *mulle_objc_object_call_inline_full( void *self, uint32_t sel, void *param)
{
   mulle_printf( "%s %p: %s\n", __FUNCTION__, self, (char *) param);
   return( self);
}


int   main( int argc, char *argv[])
{
   [(NoWarn *) 0x1848 call:"hello"];
   return( 0);
}
