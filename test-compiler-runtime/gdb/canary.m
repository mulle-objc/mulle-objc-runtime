#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>


int   main( void)
{
   printf( "pair.infraclass      = %td\n", offsetof( struct _mulle_objc_classpair, infraclass));
   printf( "pair.metaclass       = %td\n", offsetof( struct _mulle_objc_classpair, metaclass));
   printf( "pair.protocolclasses = %td\n", offsetof( struct _mulle_objc_classpair, protocolclasses));

   return( 0);
}