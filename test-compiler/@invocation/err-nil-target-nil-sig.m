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
   // Both target and sig are compile-time nil — invocation will always be nil.
   // This should produce a warning.
   SEL           sel = @selector( doSomething);
   NSInvocation *inv = @invocation( nil, sel, nil);
   (void) inv;
   return( 0);
}
