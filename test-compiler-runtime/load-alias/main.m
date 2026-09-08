#import "Consumer.h"

//
// If load-time alias resolution behaves as intended, the program aborts during
// load (before main runs) with a method-not-found for +baseValue. main() only
// exists so the program links; reaching it would itself be a failure, which the
// absence of a default.stdout (and the default.errors match) enforces.
//
int   main( void)
{
   mulle_printf( "REACHED-MAIN-UNEXPECTED\n");
   return( 0);
}
