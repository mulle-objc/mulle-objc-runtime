#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include "invocation-test.h"

#pragma clang diagnostic ignored "-Wobjc-root-class"

@class NSInvocation;
@class NSMethodSignature;

@interface Foo
- (int) add:(int) a to:(int) b;
@end

void test( id obj)
{
   NSInvocation *inv = @invocation( obj, - (int) add:(int) a to:(int) b, 1, 2)
   {
      return( a + b);
   };
   (void) inv;
}

int   main( void)
{
   return( 0);
}
