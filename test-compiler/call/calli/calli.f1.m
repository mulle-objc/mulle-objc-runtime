#define __MULLE_NO_TPS__
#define __MULLE_NO_FCS__
#define __MULLE_NO_TAO__

#define MULLE_OBJC_RUNTIME_VERSION_MAJOR  0
#define MULLE_OBJC_RUNTIME_VERSION_MINOR  26
#define MULLE_OBJC_RUNTIME_VERSION_PATCH  0
#define MULLE_OBJC_RUNTIME_LOAD_VERSION   18

#include <stdio.h>
#include <stdint.h>

@interface NoWarn
- (void) call:(char *) s;
@end

//
// test that the compiler emits the proper code, with various optimization
// levels and flags
//
void   *mulle_objc_object_call( void *self, uint32_t sel, void *param)
{
   printf( "%s %p: %s\n", __FUNCTION__, self, (char *) param);
   return( self);
}


void   *mulle_objc_object_call_inline_minimal( void *self, uint32_t sel, void *param)
{
   printf( "%s %p: %s\n", __FUNCTION__, self, (char *) param);
   return( self);
}


void   *mulle_objc_object_call_inline_partial( void *self, uint32_t sel, void *param)
{
   printf( "%s %p: %s\n", __FUNCTION__, self, (char *) param);
   return( self);
}


void   *mulle_objc_object_call_inline( void *self, uint32_t sel, void *param)
{
   printf( "%s %p: %s\n", __FUNCTION__, self, (char *) param);
   return( self);
}


void   *mulle_objc_object_call_inline_full( void *self, uint32_t sel, void *param)
{
   printf( "%s %p: %s\n", __FUNCTION__, self, (char *) param);
   return( self);
}


int   main( int argc, char *argv[])
{
   [(NoWarn *) 0x1848 call:"hello"];
   return( 0);
}
