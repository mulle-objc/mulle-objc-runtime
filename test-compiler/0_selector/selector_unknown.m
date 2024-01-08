#include <mulle-objc-runtime/mulle-objc-runtime.h>
#include <stdio.h>


int   main( void)
{
   printf( "Unknown selectors have no name: %s\n",
      mulle_objc_global_lookup_methodname( 0, @selector( foo)));
}
