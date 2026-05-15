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
   /* Prototype form produces same result as selector form when method is declared */
   void  *sig1 = @signature( bar:);
   void  *sig2 = @signature( -(void) bar:(int) x);
   printf( "same: %s\n", (sig1 == sig2) ? "ok" : "mismatch");

   /* Prototype form without prior declaration */
   void  *sig3 = @signature( -(int) add:(int) a to:(int) b);
   void  *sig4 = @signature( add:to:);
   printf( "multi: %s\n", (sig3 == sig4) ? "ok" : "mismatch");

   /* Class method prototype */
   void  *sig5 = @signature( +(id) make);
   void  *sig6 = @signature( make);
   printf( "class: %s\n", (sig5 == sig6) ? "ok" : "mismatch");

   return( 0);
}
