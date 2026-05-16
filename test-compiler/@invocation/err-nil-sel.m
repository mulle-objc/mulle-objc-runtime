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
   Foo          *obj = (Foo *) 0; // non-nil would work, but we want compile-time nil sel
   NSInvocation *inv = @invocation( obj, (SEL) 0, nil);
   (void) inv;
   return( 0);
}
