#include <mulle-objc-runtime/mulle-objc-runtime.h>


int   main( void)
{
   // @signature with a selector that has never been declared should error
   void  *sig = @signature( notDeclaredAtAll);
   (void) sig;
   return( 0);
}
