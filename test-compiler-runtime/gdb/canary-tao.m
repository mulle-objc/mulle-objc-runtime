#define MULLE_OBJC_TAO_OBJECT_HEADER 1

#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>


int   main( void)
{
   mulle_printf( "pair.infraclass      = %td\n", offsetof( struct _mulle_objc_classpair, infraclass));
   mulle_printf( "pair.metaclass       = %td\n", offsetof( struct _mulle_objc_classpair, metaclass));
   mulle_printf( "pair.mixins = %td\n", offsetof( struct _mulle_objc_classpair, mixins));

   return( 0);
}