#include <mulle-objc-runtime/mulle-objc-runtime.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"


// Two unrelated interfaces declare the same selector with different return types.
// The type encodings differ, so @signature must error instead of guessing.

@interface Foo
- (int)  conflict:(int) x;
@end

@interface Bar
- (void) conflict:(int) x;
@end


int   main( void)
{
   void  *sig = @signature( conflict:);
   (void) sig;
   return( 0);
}
