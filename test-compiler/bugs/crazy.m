#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"
#pragma clang diagnostic ignored "-Wobjc-method-access"


@implementation Foo

- (id) initWithXXX:(id) aString
     relativeToURL:(id) url
{
   return( [aString self]);
}

@end


int  main( void)
{
   return( 0);
}
