#include <mulle-objc-runtime/mulle-objc-runtime.h>


int   main( void)
{
   printf( "%s\n",
      sizeof( mulle_objc_uniqueid_t) == sizeof( SEL) ? "YES" : "NO");
}
