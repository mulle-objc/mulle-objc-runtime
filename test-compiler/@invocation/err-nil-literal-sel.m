#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include "invocation-test.h"

#pragma clang diagnostic ignored "-Wobjc-root-class"

@class NSInvocation;
@class NSMethodSignature;


@interface Foo
- (void) doSomething;
@end


int   main( void)
{
   // nil literal as selector — also a compile-time null constant
   NSInvocation *inv = @invocation( [Foo new], nil, nil);
   (void) inv;
   return( 0);
}
