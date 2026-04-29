#include <mulle-objc-runtime/mulle-objc-runtime.h>


@protocol_interface X

@required
- (void) doFuture;

@optional
- (void) doSomething;
- (void) doOther;
@end


@protocol_implementation X

- (void) doSomething 
{
}

@end

int main( void) 
{ 
   return 0; 
}
