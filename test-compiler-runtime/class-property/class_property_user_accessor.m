#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"
#pragma clang diagnostic ignored "-Wobjc-property-implementation"

//
// Test that user-provided accessor suppresses synthesis
//
@interface Baz
@property (class, assign) int custom;
@end

static int   _custom_backing = 100;

@implementation Baz

+ (int) custom
{
   // user-provided getter — should NOT be replaced by synthesis
   return( _custom_backing);
}

+ (void) setCustom:(int) v
{
   // user-provided setter
   _custom_backing = v * 2;
}

@end

int   main( void)
{
   // user getter returns 100 (not self->_custom which is 0)
   if( [Baz custom] != 100)
   {
      printf( "FAIL: user getter not used, got %d\n", [Baz custom]);
      return( 1);
   }

   [Baz setCustom:5];
   // user setter doubles the value
   if( [Baz custom] != 10)
   {
      printf( "FAIL: user setter not used, got %d\n", [Baz custom]);
      return( 1);
   }

   return( 0);
}
