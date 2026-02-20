#include <mulle-objc-runtime/mulle-objc-runtime.h>


int   main( void)
{
   mulle_printf( "%s\n",
      sizeof( mulle_objc_protocolid_t) == sizeof( PROTOCOL) ? "YES" : "NO");
}
