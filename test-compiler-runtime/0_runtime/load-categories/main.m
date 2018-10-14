#include <mulle-objc-runtime/mulle-objc-runtime.h>


int   main( int argc, char *argv[])
{
   if( mulle_objc_global_check_universe( __MULLE_OBJC_UNIVERSENAME__) != mulle_objc_universe_is_ok)
      return( 1);
   // just look for +load output
   return( 0);
}
