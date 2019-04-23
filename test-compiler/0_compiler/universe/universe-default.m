#include <mulle-objc-runtime/mulle-objc-runtime.h>


int  main( void)
{
   printf( "name : %s\n",  __MULLE_OBJC_UNIVERSENAME__ ? __MULLE_OBJC_UNIVERSENAME__ : "NULL");
   printf( "id   : %lx\n", (long) __MULLE_OBJC_UNIVERSEID__);

   return( 0);
}
