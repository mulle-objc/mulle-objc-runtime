#include <mulle-objc-runtime/mulle-objc-runtime.h>


@mixin X

- (void) doSomething;

@required
- (void) doFuture;
- (void) doOther;

@end


@implementation X

- (void) doSomething 
{
}

- (void) doOther
{
}

@end

int main( void) 
{ 
   return 0; 
}
