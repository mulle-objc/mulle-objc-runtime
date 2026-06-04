#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"
#pragma clang diagnostic ignored "-Wobjc-property-implementation"
#pragma clang diagnostic ignored "-Wreceiver-expr"

//
// Test basic class property storage via self->_field and
// synthesized accessors (+sharedCount / +setSharedCount:)
//
@interface Foo
@property (class, assign) int sharedCount;
@end

@implementation Foo
+ (void) increment
{
   self->_sharedCount = self->_sharedCount + 1;
}
@end

int   main( void)
{
   // Test synthesized getter
   if( [Foo sharedCount] != 0)
   {
      printf( "FAIL: initial sharedCount != 0\n");
      return( 1);
   }

   // Test synthesized setter
   [Foo setSharedCount:10];
   if( [Foo sharedCount] != 10)
   {
      printf( "FAIL: sharedCount != 10 after set\n");
      return( 1);
   }

   // Test self->_field access in user method
   [Foo increment];
   if( [Foo sharedCount] != 11)
   {
      printf( "FAIL: sharedCount != 11 after increment\n");
      return( 1);
   }

   return( 0);
}
