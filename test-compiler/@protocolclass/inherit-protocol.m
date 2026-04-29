#include <mulle-objc-runtime/mulle-objc-runtime.h>


@protocol_interface X

- (void) doSomething;

@required
- (void) doFuture;
- (void) doOther;

@end


@protocol_implementation X

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
