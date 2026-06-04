#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"
#pragma clang diagnostic ignored "-Wobjc-property-implementation"
#pragma clang diagnostic ignored "-Wreceiver-expr"

//
// Test [self msg] dispatch in + methods and self->_field access
//
@interface Foo
@property (class, assign) int value;
+ (int) getValue;
@end

@implementation Foo
+ (int) getValue
{
   return self->_value;
}
+ (void) test
{
   self->_value = 42;
   int v = [self getValue];
   if( v != 42)
   {
      printf( "FAIL: [self getValue] returned %d, expected 42\n", v);
      abort();
   }
}
@end

int   main( void)
{
   [Foo test];
   return( 0);
}
