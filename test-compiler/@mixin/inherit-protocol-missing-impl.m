#include <mulle-objc-runtime/mulle-objc-runtime.h>


@mixin X

@required
- (void) doFuture;

@optional
- (void) doSomething;
- (void) doOther;
@end


@implementation X

- (void) doSomething 
{
}

@end

int main( void) 
{ 
   return 0; 
}
