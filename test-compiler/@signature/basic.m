#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>

#pragma clang diagnostic ignored "-Wobjc-root-class"


@interface Foo
- (void) bar:(int) x;
- (int)  add:(int) a to:(int) b;
+ (id)   make;
@end


int   main( void)
{
   void  *sig1 = @signature( bar:);
   void  *sig2 = @signature( add:to:);
   void  *sig3 = @signature( make);

   printf( "sig1: %s\n", sig1 ? "ok" : "null");
   printf( "sig2: %s\n", sig2 ? "ok" : "null");
   printf( "sig3: %s\n", sig3 ? "ok" : "null");

   return( 0);
}
