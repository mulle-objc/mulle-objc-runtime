#include <mulle-objc-runtime/mulle-objc-runtime.h>


main()
{
   printf( "%s\n",
      sizeof( mulle_objc_protocolid_t) == sizeof( PROTOCOL) ? "YES" : "NO");
}
